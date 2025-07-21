/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of conditional select instructions (CSEL, CSINC, CSINV, CSNEG, CCMN, CCMP)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P012: " fmt

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "a55_stl_constants.h"
#include "a55_stl_utils.h"
#include "a55_stl.h"

#define TEST_CSEL(in1, in2, golden, eq) \
	do { \
        uint64_t result; \
		asm volatile("msr nzcv, xzr"); \
		if (eq) { \
			asm volatile("csel %[res], %[a], %[b], eq\n" \
				: [res] "=r"(result) \
				: [a] "r"(in1), [b] "r"(in2)); \
		} else { \
			asm volatile("csel %[res], %[a], %[b], ne\n" \
				: [res] "=r"(result) \
				: [a] "r"(in1), [b] "r"(in2)); \
		} \
        if (result != golden) { \
			pr_err("[%d] CSEL: got=0x%llx, expect=0x%llx, in1=0x%llx, in2=0x%llx, eq=%d\n", \
				__LINE__, result, (uint64_t)golden, (uint64_t)(in1), (uint64_t)(in2), (int)(eq)); \
            return -1; \
        } \
	} while (0)

#define TEST_CSINC(in1, in2, golden, eq) \
	do { \
        uint64_t result; \
		asm volatile("msr nzcv, xzr"); \
		if (eq) { \
			asm volatile("csinc %[res], %[a], %[b], eq\n" \
				: [res] "=r"(result) \
				: [a] "r"(in1), [b] "r"(in2)); \
		} else { \
			asm volatile("csinc %[res], %[a], %[b], ne\n" \
				: [res] "=r"(result) \
				: [a] "r"(in1), [b] "r"(in2)); \
		} \
        if (result != golden) { \
			pr_err("[%d] CSINC: got=0x%llx, expect=0x%llx\n", __LINE__, result, (uint64_t)golden); \
            return -1; \
        } \
	} while (0)

#define TEST_CSINV(in1, in2, golden, eq) \
	do { \
        uint64_t result; \
		asm volatile("msr nzcv, xzr"); \
		if (eq) { \
			asm volatile("csinv %[res], %[a], %[b], eq\n" \
				: [res] "=r"(result) \
				: [a] "r"(in1), [b] "r"(in2)); \
		} else { \
			asm volatile("csinv %[res], %[a], %[b], ne\n" \
				: [res] "=r"(result) \
				: [a] "r"(in1), [b] "r"(in2)); \
		} \
        if (result != golden) { \
			pr_err("[%d] CSINV: got=0x%llx, expect=0x%llx, in1=0x%llx, in2=0x%llx, eq=%d\n", \
				__LINE__, result, (uint64_t)golden, (uint64_t)(in1), (uint64_t)(in2), (int)(eq)); \
            return -1; \
        } \
	} while (0)

#define TEST_CSNEG(in1, in2, golden, eq) \
	do { \
        uint64_t result; \
		asm volatile("msr nzcv, xzr"); \
		if (eq) { \
			asm volatile("csneg %[res], %[a], %[b], eq\n" \
				: [res] "=r"(result) \
				: [a] "r"(in1), [b] "r"(in2)); \
		} else { \
			asm volatile("csneg %[res], %[a], %[b], ne\n" \
				: [res] "=r"(result) \
				: [a] "r"(in1), [b] "r"(in2)); \
		} \
        if (result != golden) { \
			pr_err("[%d] CSNEG: got=0x%llx, expect=0x%llx, in1=0x%llx, in2=0x%llx, eq=%d\n", \
				__LINE__, result, (uint64_t)golden, (uint64_t)(in1), (uint64_t)(in2), (int)(eq)); \
            return -1; \
        } \
	} while (0)

static inline int test_ccmn_case1(uint64_t imm1, uint64_t imm2, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x26 = A55_STL_BM62_INPUT_VALUE_1;
    uint64_t x2  = A55_STL_BM62_INPUT_VALUE_2;
    uint64_t x3  = A55_STL_BM62_INPUT_VALUE_1;
    uint64_t x4  = A55_STL_BM62_INPUT_VALUE_2;
    uint64_t x5  = A55_STL_BM62_INPUT_VALUE_3;
    uint64_t x6  = A55_STL_BM62_INPUT_VALUE_2;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmn %[x26], %[x2], %[imm_val1], ne\n"
        "ccmn %[x3], %[x4], %[imm_val1], ne\n"
        "ccmn %[x5], %[x6], %[imm_val2], ge\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags)
        : [x26] "r"(x26), [x2] "r"(x2), [x3] "r"(x3), [x4] "r"(x4),
          [x5] "r"(x5), [x6] "r"(x6),
          [imm_val1] "I"(imm1), [imm_val2] "I"(imm2)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMN-1: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}

#define TEST_CCMN1(imm1, imm2, golden_flags) \
	do { \
		if (test_ccmn_case1((imm1), (imm2), (golden_flags)) < 0) { \
			return -1; \
		} \
	} while (0)

static inline int test_ccmn_case2(uint64_t imm1, uint64_t imm2, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x7  = A55_STL_BM62_INPUT_VALUE_1;
    uint64_t x8  = A55_STL_BM62_INPUT_VALUE_2;
    uint64_t x9  = A55_STL_BM62_INPUT_VALUE_3;
    uint64_t x10 = A55_STL_BM62_INPUT_VALUE_2;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmn %[x7], %[x8], %[imm_val1], ne\n"
        "ccmn %[x9], %[x10], %[imm_val2], ge\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags)
        : [x7] "r"(x7), [x8] "r"(x8), [x9] "r"(x9), [x10] "r"(x10),
          [imm_val1] "I"(imm1), [imm_val2] "I"(imm2)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMN-2: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}

#define TEST_CCMN2(imm1, imm2, golden_flags) \
	do { \
		if (test_ccmn_case2((imm1), (imm2), (golden_flags)) < 0) { \
			return -1; \
		} \
	} while (0)

static inline int test_ccmn_case3(uint64_t imm1, uint64_t imm2, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x11 = A55_STL_BM62_INPUT_VALUE_4;
    uint64_t x12 = A55_STL_BM62_INPUT_VALUE_2;
    uint64_t x13 = A55_STL_BM62_INPUT_VALUE_4;
    uint64_t x14 = A55_STL_BM62_INPUT_VALUE_2;
    uint64_t x15 = A55_STL_BM62_INPUT_VALUE_5;
    uint64_t x16 = A55_STL_BM62_INPUT_VALUE_2;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmn %[x11], %[x12], %[imm_val1], ne\n"
        "ccmn %[x13], %[x14], %[imm_val1], ne\n"
        "ccmn %[x15], %[x16], %[imm_val2], eq\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags)
        : [x11] "r"(x11), [x12] "r"(x12), [x13] "r"(x13), [x14] "r"(x14),
          [x15] "r"(x15), [x16] "r"(x16),
          [imm_val1] "I"(imm1), [imm_val2] "I"(imm2)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMN-3: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMN3(imm1, imm2, golden_flags) \
	do { \
		if (test_ccmn_case3((imm1), (imm2), (golden_flags)) < 0) { \
			return -1; \
		} \
	} while (0)

static inline int test_ccmn_case4(uint64_t imm1, uint64_t imm2, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x17 = A55_STL_BM62_INPUT_VALUE_4;
    uint64_t x18 = A55_STL_BM62_INPUT_VALUE_2;
    uint64_t x19 = A55_STL_BM62_INPUT_VALUE_5;
    uint64_t x20 = A55_STL_BM62_INPUT_VALUE_2;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmn %[x17], %[x18], %[imm_val1], ne\n"
        "ccmn %[x19], %[x20], %[imm_val2], eq\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags)
        : [x17] "r"(x17), [x18] "r"(x18), [x19] "r"(x19), [x20] "r"(x20),
          [imm_val1] "I"(imm1), [imm_val2] "I"(imm2)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMN-4: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMN4(imm1, imm2, golden_flags) \
	do { \
		if (test_ccmn_case4((imm1), (imm2), (golden_flags)) < 0) { \
			return -1; \
		} \
	} while (0)

static inline int test_ccmp_case1(uint64_t imm1, uint64_t imm2, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x26 = A55_STL_BM63_INPUT_VALUE_1;
    uint64_t x2  = A55_STL_BM63_INPUT_VALUE_2;
    uint64_t x3  = A55_STL_BM63_INPUT_VALUE_1;
    uint64_t x4  = A55_STL_BM63_INPUT_VALUE_2;
    uint64_t x5  = A55_STL_BM63_INPUT_VALUE_3;
    uint64_t x6  = A55_STL_BM63_INPUT_VALUE_2;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmp %[x26], %[x2], %[imm_val1], eq\n"
        "ccmp %[x3], %[x4], %[imm_val1], eq\n"
        "ccmp %[x5], %[x6], %[imm_val2], ne\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags)
        : [x26] "r"(x26), [x2] "r"(x2), [x3] "r"(x3), [x4] "r"(x4),
          [x5] "r"(x5), [x6] "r"(x6),
          [imm_val1] "I"(imm1), [imm_val2] "I"(imm2)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMP-1: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMP1(imm1, imm2, golden_flags) \
    do { \
        if (test_ccmp_case1((imm1), (imm2), (golden_flags)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_ccmp_case2(uint64_t imm1, uint64_t imm2, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x7  = A55_STL_BM63_INPUT_VALUE_1;
    uint64_t x8  = A55_STL_BM63_INPUT_VALUE_2;
    uint64_t x9  = A55_STL_BM63_INPUT_VALUE_3;
    uint64_t x10 = A55_STL_BM63_INPUT_VALUE_2;
    uint64_t x3  = A55_STL_BM63_INPUT_VALUE_1;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmp %[x7], %[x8], %[imm_val1], eq\n"
        "ccmp %[x9], %[x10], %[imm_val2], ne\n"
        "mov %[x26], %[x3]\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags), [x26] "=r"(x7)
        : [x7] "r"(x7), [x8] "r"(x8), [x9] "r"(x9), [x10] "r"(x10),
          [imm_val1] "I"(imm1), [imm_val2] "I"(imm2), [x3] "r"(x3)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMP-2: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMP2(imm1, imm2, golden_flags) \
    do { \
        if (test_ccmp_case2((imm1), (imm2), (golden_flags)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_ccmp_case3(uint64_t imm1, uint64_t imm2, uint64_t imm3, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x11 = A55_STL_BM63_INPUT_VALUE_4;
    uint64_t x12 = A55_STL_BM63_INPUT_VALUE_5;
    uint64_t x13 = A55_STL_BM63_INPUT_VALUE_4;
    uint64_t x14 = A55_STL_BM63_INPUT_VALUE_5;
    uint64_t x15 = A55_STL_BM63_INPUT_VALUE_6;
    uint64_t x16 = A55_STL_BM63_INPUT_VALUE_5;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmp %[x11], %[x12], %[imm_val1], eq\n"
        "ccmp %[x13], %[x14], %[imm_val1], eq\n"
        "ccmp %[x15], %[x16], %[imm_val3], eq\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags)
        : [x11] "r"(x11), [x12] "r"(x12), [x13] "r"(x13), [x14] "r"(x14),
          [x15] "r"(x15), [x16] "r"(x16),
          [imm_val1] "I"(imm1), [imm_val3] "I"(imm3)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMP-3: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMP3(imm1, imm3, golden_flags) \
    do { \
        if (test_ccmp_case3((imm1), (imm1), (imm3), (golden_flags)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_ccmp_case4(uint64_t imm1, uint64_t imm3, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x17 = A55_STL_BM63_INPUT_VALUE_4;
    uint64_t x18 = A55_STL_BM63_INPUT_VALUE_5;
    uint64_t x19 = A55_STL_BM63_INPUT_VALUE_6;
    uint64_t x20 = A55_STL_BM63_INPUT_VALUE_5;
    uint64_t x13 = A55_STL_BM63_INPUT_VALUE_4;
    asm volatile(
        "msr nzcv, xzr\n"
        "ccmp %[x17], %[x18], %[imm_val1], eq\n"
        "ccmp %[x19], %[x20], %[imm_val3], eq\n"
        "mov %[x11], %[x13]\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags), [x11] "=r"(x17)
        : [x17] "r"(x17), [x18] "r"(x18), [x19] "r"(x19), [x20] "r"(x20),
          [imm_val1] "I"(imm1), [imm_val3] "I"(imm3), [x13] "r"(x13)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMP-4: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMP4(imm1, imm3, golden_flags) \
    do { \
        if (test_ccmp_case4((imm1), (imm3), (golden_flags)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_ccmp_imm_case1(uint64_t imm1, uint64_t imm2, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x26 = A55_STL_BM64_INPUT_VALUE_1;
    uint64_t x3  = A55_STL_BM64_INPUT_VALUE_1;
    uint64_t x5  = A55_STL_BM64_INPUT_VALUE_3;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmp %[x26], %[imm2], %[imm1], eq\n"
        "ccmp %[x3], %[imm2], %[imm1], eq\n"
        "ccmp %[x5], %[imm2], %[imm2], ne\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags)
        : [x26] "r"(x26), [x3] "r"(x3), [x5] "r"(x5),
          [imm1] "I"(imm1), [imm2] "I"(imm2)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMP-IMM-1: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMP_IMM1(imm1, imm2, golden_flags) \
    do { \
        if (test_ccmp_imm_case1((imm1), (imm2), (golden_flags)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_ccmp_imm_case2(uint64_t imm1, uint64_t imm2, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x7  = A55_STL_BM64_INPUT_VALUE_1;
    uint64_t x9  = A55_STL_BM64_INPUT_VALUE_3;
    uint64_t x3  = A55_STL_BM64_INPUT_VALUE_1;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmp %[x7], %[imm2], %[imm1], eq\n"
        "ccmp %[x9], %[imm2], %[imm2], ne\n"
        "mov %[x26], %[x3]\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags), [x26] "=r"(x7)
        : [x7] "r"(x7), [x9] "r"(x9),
          [imm1] "I"(imm1), [imm2] "I"(imm2), [x3] "r"(x3)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMP-IMM-2: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMP_IMM2(imm1, imm2, golden_flags) \
    do { \
        if (test_ccmp_imm_case2((imm1), (imm2), (golden_flags)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_ccmp_imm_case3(uint64_t imm1, uint64_t imm3, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x11 = A55_STL_BM64_INPUT_VALUE_4;
    uint64_t x13 = A55_STL_BM64_INPUT_VALUE_4;
    uint64_t x15 = A55_STL_BM64_INPUT_VALUE_6;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmp %[x11], %[imm5], %[imm1], eq\n"
        "ccmp %[x13], %[imm5], %[imm1], eq\n"
        "ccmp %[x15], %[imm5], %[imm3], eq\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags)
        : [x11] "r"(x11), [x13] "r"(x13), [x15] "r"(x15),
          [imm1] "I"(imm1), [imm3] "I"(imm3), [imm5] "I"(A55_STL_BM64_INPUT_VALUE_5)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMP-IMM-3: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMP_IMM3(imm1, imm3, golden_flags) \
    do { \
        if (test_ccmp_imm_case3((imm1), (imm3), (golden_flags)) < 0) { \
            return -1; \
        } \
    } while (0)

static inline int test_ccmp_imm_case4(uint64_t imm1, uint64_t imm3, uint64_t golden_flags)
{
    uint64_t flags;
    uint64_t x17 = A55_STL_BM64_INPUT_VALUE_4;
    uint64_t x19 = A55_STL_BM64_INPUT_VALUE_6;
    uint64_t x13 = A55_STL_BM64_INPUT_VALUE_4;

    asm volatile(
        "msr nzcv, xzr\n"
        "ccmp %[x17], %[imm5], %[imm1], eq\n"
        "ccmp %[x19], %[imm5], %[imm3], eq\n"
        "mov %[x11], %[x13]\n"
        "mrs %[f], nzcv\n"
        : [f] "=r"(flags), [x11] "=r"(x17)
        : [x17] "r"(x17), [x19] "r"(x19),
          [imm1] "I"(imm1), [imm3] "I"(imm3), [imm5] "I"(A55_STL_BM64_INPUT_VALUE_5), [x13] "r"(x13)
    );
    if (flags != golden_flags) {
        pr_err("[%d] CCMP-IMM-4: got_nzcv=0x%llx, expect_nzcv=0x%llx\n", __LINE__, flags, golden_flags);
        return -1;
    }
    return 0;
}
#define TEST_CCMP_IMM4(imm1, imm3, golden_flags) \
    do { \
        if (test_ccmp_imm_case4((imm1), (imm3), (golden_flags)) < 0) { \
            return -1; \
        } \
    } while (0)

int a55_stl_core_diagnose_p012_n001_c_impl(void) {
	// Basic Module 58
	// CSEL <Xd>, <Xn>, <Xm>, <cond>
	TEST_CSEL(A55_STL_BM58_INPUT_VALUE_1, A55_STL_BM58_INPUT_VALUE_2, A55_STL_BM58_GOLDEN_VALUE_1, 0);
	TEST_CSEL(A55_STL_BM58_INPUT_VALUE_3, A55_STL_BM58_INPUT_VALUE_4, A55_STL_BM58_GOLDEN_VALUE_2, 1);
	TEST_CSEL(A55_STL_BM58_INPUT_VALUE_5, A55_STL_BM58_INPUT_VALUE_6, A55_STL_BM58_GOLDEN_VALUE_3, 0);
	TEST_CSEL(A55_STL_BM58_INPUT_VALUE_7, A55_STL_BM58_INPUT_VALUE_8, A55_STL_BM58_GOLDEN_VALUE_4, 1);

	TEST_CSINC(A55_STL_BM59_INPUT_VALUE_1, A55_STL_BM59_INPUT_VALUE_2, A55_STL_BM59_GOLDEN_VALUE_1, 0);
	TEST_CSINC(A55_STL_BM59_INPUT_VALUE_3, A55_STL_BM59_INPUT_VALUE_4, A55_STL_BM59_GOLDEN_VALUE_2, 1);
	TEST_CSINC(A55_STL_BM59_INPUT_VALUE_5, A55_STL_BM59_INPUT_VALUE_5, A55_STL_BM59_GOLDEN_VALUE_3, 1);
	TEST_CSINC(A55_STL_BM59_INPUT_VALUE_6, A55_STL_BM59_INPUT_VALUE_6, A55_STL_BM59_GOLDEN_VALUE_4, 0);
	TEST_CSINC(0, 0, A55_STL_BM59_GOLDEN_VALUE_5, 1);
	TEST_CSINC(0, 0, A55_STL_BM59_GOLDEN_VALUE_6, 0);

	TEST_CSINV(A55_STL_BM60_INPUT_VALUE_1, A55_STL_BM60_INPUT_VALUE_2, A55_STL_BM60_GOLDEN_VALUE_1, 0);
	TEST_CSINV(A55_STL_BM60_INPUT_VALUE_3, A55_STL_BM60_INPUT_VALUE_4, A55_STL_BM60_GOLDEN_VALUE_2, 1);
	TEST_CSINV(A55_STL_BM60_INPUT_VALUE_5, A55_STL_BM60_INPUT_VALUE_5, A55_STL_BM60_GOLDEN_VALUE_3, 1);
	TEST_CSINV(A55_STL_BM60_INPUT_VALUE_6, A55_STL_BM60_INPUT_VALUE_6, A55_STL_BM60_GOLDEN_VALUE_3, 0);
	TEST_CSINV(0, 0, A55_STL_BM60_GOLDEN_VALUE_4, 1);
	TEST_CSINV(0, 0, A55_STL_BM60_GOLDEN_VALUE_5, 0);

    // Basic Module 61
    // CSNEG <Xd>, <Xn>, <Xm>, <cond>
    TEST_CSNEG(A55_STL_BM61_INPUT_VALUE_1, A55_STL_BM61_INPUT_VALUE_2, A55_STL_BM61_GOLDEN_VALUE_1, 0);
    TEST_CSNEG(A55_STL_BM61_INPUT_VALUE_1, A55_STL_BM61_INPUT_VALUE_2, A55_STL_BM61_GOLDEN_VALUE_1, 0);
    TEST_CSNEG(A55_STL_BM61_INPUT_VALUE_3, A55_STL_BM61_INPUT_VALUE_4, A55_STL_BM61_GOLDEN_VALUE_2, 1);
    TEST_CSNEG(A55_STL_BM61_INPUT_VALUE_3, A55_STL_BM61_INPUT_VALUE_4, A55_STL_BM61_GOLDEN_VALUE_2, 1);
    TEST_CSNEG(A55_STL_BM61_INPUT_VALUE_5, A55_STL_BM61_INPUT_VALUE_5, A55_STL_BM61_GOLDEN_VALUE_3, 1);
    TEST_CSNEG(A55_STL_BM61_INPUT_VALUE_5, A55_STL_BM61_INPUT_VALUE_5, A55_STL_BM61_GOLDEN_VALUE_3, 1);
    TEST_CSNEG(A55_STL_BM61_INPUT_VALUE_6, A55_STL_BM61_INPUT_VALUE_6, A55_STL_BM61_GOLDEN_VALUE_4, 0);
    TEST_CSNEG(A55_STL_BM61_INPUT_VALUE_6, A55_STL_BM61_INPUT_VALUE_6, A55_STL_BM61_GOLDEN_VALUE_4, 0);

    // Basic Module 62
    // CCMN <Xn>, <Xm>, #<nzcv>, <cond>
	TEST_CCMN1(A55_STL_BM62_IMM_VALUE_1, A55_STL_BM62_IMM_VALUE_2, A55_STL_BM62_GOLDEN_FLAGS_1);
	TEST_CCMN2(A55_STL_BM62_IMM_VALUE_1, A55_STL_BM62_IMM_VALUE_2, A55_STL_BM62_GOLDEN_FLAGS_1);
	TEST_CCMN3(A55_STL_BM62_IMM_VALUE_1, A55_STL_BM62_IMM_VALUE_3, A55_STL_BM62_GOLDEN_FLAGS_2);
	TEST_CCMN4(A55_STL_BM62_IMM_VALUE_1, A55_STL_BM62_IMM_VALUE_3, A55_STL_BM62_GOLDEN_FLAGS_2);


    // Basic Module 63
    // CCMP <Xn>, <Xm>, #<nzcv>, <cond>
    TEST_CCMP1(A55_STL_BM63_IMM_VALUE_1, A55_STL_BM63_IMM_VALUE_2, A55_STL_BM63_GOLDEN_FLAGS_1);
    TEST_CCMP2(A55_STL_BM63_IMM_VALUE_1, A55_STL_BM63_IMM_VALUE_2, A55_STL_BM63_GOLDEN_FLAGS_1);
    TEST_CCMP3(A55_STL_BM63_IMM_VALUE_1, A55_STL_BM63_IMM_VALUE_3, A55_STL_BM63_GOLDEN_FLAGS_2);
    TEST_CCMP4(A55_STL_BM63_IMM_VALUE_1, A55_STL_BM63_IMM_VALUE_3, A55_STL_BM63_GOLDEN_FLAGS_2);

    // Basic Module 64
    // CCMP <Xn>, #<imm>, #<nzcv>, <cond>
    TEST_CCMP_IMM1(A55_STL_BM64_IMM_VALUE_1, A55_STL_BM64_IMM_VALUE_2, A55_STL_BM64_GOLDEN_FLAGS_1);
    TEST_CCMP_IMM2(A55_STL_BM64_IMM_VALUE_1, A55_STL_BM64_IMM_VALUE_2, A55_STL_BM64_GOLDEN_FLAGS_1);
    TEST_CCMP_IMM3(A55_STL_BM64_IMM_VALUE_1, A55_STL_BM64_IMM_VALUE_3, A55_STL_BM64_GOLDEN_FLAGS_2);
    TEST_CCMP_IMM4(A55_STL_BM64_IMM_VALUE_1, A55_STL_BM64_IMM_VALUE_3, A55_STL_BM64_GOLDEN_FLAGS_2);

    return 0;
}

void a55_stl_core_diagnose_p012(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p012_n001_c_impl() != 0) {
        pr_info("A55 STL Core P012: Test failed\n");
    } else {
        pr_info("A55 STL Core P012: Test passed\n");
    }
}