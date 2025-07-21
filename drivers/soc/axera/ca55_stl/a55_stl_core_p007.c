/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of logical instructions BIC, BICS, EON (shift), EOR (shift), ORR (shift), ORN (shift)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P007: " fmt

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

// BIC (shift)
#define TEST_BIC_SHIFT(in1, in2, shift_type, shift_amt, golden) \
    do { \
        uint64_t result; \
        if (shift_type == 0) { \
            asm volatile("bic %[res], %[a], %[b], asr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 1) { \
            asm volatile("bic %[res], %[a], %[b], lsr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 2) { \
            asm volatile("bic %[res], %[a], %[b], lsl %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 3) { \
            asm volatile("bic %[res], %[a], %[b], ror %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } \
        if (result != golden) { \
            pr_err("[%d] BIC_SHIFT: in1=0x%llx, in2=0x%llx, type=%d, amt=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, shift_type, shift_amt, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// BICS (shift)
#define TEST_BICS_SHIFT(in1, in2, shift_type, shift_amt, golden, golden_flags) \
    do { \
        uint64_t result, flags64; \
        asm volatile("msr nzcv, xzr\n" ::: "cc"); \
        if (shift_type == 2) { \
            asm volatile("bics %[res], %[a], %[b], lsl %[amt]\n" \
                         "mrs %[flg], nzcv\n" \
                : [res] "=&r"(result), [flg] "=&r"(flags64) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt) \
                : "cc"); \
        } else if (shift_type == 1) { \
            asm volatile("bics %[res], %[a], %[b], lsr %[amt]\n" \
                         "mrs %[flg], nzcv\n" \
                : [res] "=&r"(result), [flg] "=&r"(flags64) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt) \
                : "cc"); \
        } \
        uint32_t flags = (uint32_t)flags64; \
        if ((result != golden) && (flags != (uint32_t)(golden_flags))) { \
            pr_err("[%d] BICS_SHIFT: in1=0x%llx, in2=0x%llx, type=%d, amt=%d, got=0x%llx/0x%08x, expect=0x%llx/0x%08x\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, shift_type, shift_amt, result, flags, (uint64_t)golden, (uint32_t)(golden_flags)); \
            return -1; \
        } \
    } while (0)

// EON (shift)
#define TEST_EON_SHIFT(in1, in2, shift_type, shift_amt, golden) \
    do { \
        uint64_t result; \
        if (shift_type == 0) { \
            asm volatile("eon %[res], %[a], %[b], asr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 1) { \
            asm volatile("eon %[res], %[a], %[b], lsr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 2) { \
            asm volatile("eon %[res], %[a], %[b], lsl %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 3) { \
            asm volatile("eon %[res], %[a], %[b], ror %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } \
        if (result != golden) { \
            pr_err("[%d] EON_SHIFT: in1=0x%llx, in2=0x%llx, type=%d, amt=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, shift_type, shift_amt, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// EOR (shift)
#define TEST_EOR_SHIFT(in1, in2, shift_type, shift_amt, golden) \
    do { \
        uint64_t result; \
        if (shift_type == 0) { \
            asm volatile("eor %[res], %[a], %[b], asr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 1) { \
            asm volatile("eor %[res], %[a], %[b], lsr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 2) { \
            asm volatile("eor %[res], %[a], %[b], lsl %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 3) { \
            asm volatile("eor %[res], %[a], %[b], ror %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } \
        if (result != golden) { \
            pr_err("[%d] EOR_SHIFT: in1=0x%llx, in2=0x%llx, type=%d, amt=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, shift_type, shift_amt, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// ORR (shift)
#define TEST_ORR_SHIFT(in1, in2, shift_type, shift_amt, golden) \
    do { \
        uint64_t result; \
        if (shift_type == 2) { \
            asm volatile("orr %[res], %[a], %[b], lsl %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 1) { \
            asm volatile("orr %[res], %[a], %[b], lsr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } \
        if (result != golden) { \
            pr_err("[%d] ORR_SHIFT: in1=0x%llx, in2=0x%llx, type=%d, amt=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, shift_type, shift_amt, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

// ORN (shift)
#define TEST_ORN_SHIFT(in1, in2, shift_type, shift_amt, golden) \
    do { \
        uint64_t result; \
        if (shift_type == 2) { \
            asm volatile("orn %[res], %[a], %[b], lsl %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } else if (shift_type == 1) { \
            asm volatile("orn %[res], %[a], %[b], lsr %[amt]\n" \
                : [res] "=r"(result) \
                : [a] "r"(in1), [b] "r"(in2), [amt] "I"(shift_amt)); \
        } \
        if (result != golden) { \
            pr_err("[%d] ORN_SHIFT: in1=0x%llx, in2=0x%llx, type=%d, amt=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)in1, (uint64_t)in2, shift_type, shift_amt, result, (uint64_t)golden); \
            return -1; \
        } \
    } while (0)

int a55_stl_core_diagnose_p007_n001_c_impl(void) {
    // BIC (shift)
    TEST_BIC_SHIFT(A55_STL_BM34_INPUT_VALUE_1, A55_STL_BM34_INPUT_VALUE_2, 0, A55_STL_BM34_IMM_VALUE_1, A55_STL_BM34_GOLDEN_VALUE_1);
    TEST_BIC_SHIFT(A55_STL_BM34_INPUT_VALUE_3, A55_STL_BM34_INPUT_VALUE_4, 1, A55_STL_BM34_IMM_VALUE_2, A55_STL_BM34_GOLDEN_VALUE_2);
    TEST_BIC_SHIFT(A55_STL_BM34_INPUT_VALUE_5, A55_STL_BM34_INPUT_VALUE_6, 2, A55_STL_BM34_IMM_VALUE_3, A55_STL_BM34_GOLDEN_VALUE_3);
    TEST_BIC_SHIFT(A55_STL_BM34_INPUT_VALUE_7, A55_STL_BM34_INPUT_VALUE_8, 3, A55_STL_BM34_IMM_VALUE_4, A55_STL_BM34_GOLDEN_VALUE_4);

    // BICS (shift)
    TEST_BICS_SHIFT(A55_STL_BM35_INPUT_VALUE_3, A55_STL_BM35_INPUT_VALUE_4, 2, A55_STL_BM35_IMM_VALUE_1, A55_STL_BM35_GOLDEN_VALUE_1, A55_STL_BM35_GOLDEN_FLAGS_1);
    TEST_BICS_SHIFT(A55_STL_BM35_INPUT_VALUE_1, A55_STL_BM35_INPUT_VALUE_2, 2, A55_STL_BM35_IMM_VALUE_1, A55_STL_BM35_GOLDEN_VALUE_2, A55_STL_BM35_GOLDEN_FLAGS_1);
    TEST_BICS_SHIFT(A55_STL_BM35_INPUT_VALUE_7, A55_STL_BM35_INPUT_VALUE_4, 1, A55_STL_BM35_IMM_VALUE_1, A55_STL_BM35_GOLDEN_VALUE_3, A55_STL_BM35_GOLDEN_FLAGS_2);
    TEST_BICS_SHIFT(A55_STL_BM35_INPUT_VALUE_5, A55_STL_BM35_INPUT_VALUE_6, 1, A55_STL_BM35_IMM_VALUE_2, A55_STL_BM35_GOLDEN_VALUE_4, A55_STL_BM35_GOLDEN_FLAGS_2);

    // EON (shift)
    TEST_EON_SHIFT(A55_STL_BM36_INPUT_VALUE_1, A55_STL_BM36_INPUT_VALUE_2, 0, A55_STL_BM36_IMM_VALUE_1, A55_STL_BM36_GOLDEN_VALUE_1);
    TEST_EON_SHIFT(A55_STL_BM36_INPUT_VALUE_3, A55_STL_BM36_INPUT_VALUE_4, 1, A55_STL_BM36_IMM_VALUE_2, A55_STL_BM36_GOLDEN_VALUE_2);
    TEST_EON_SHIFT(A55_STL_BM36_INPUT_VALUE_5, A55_STL_BM36_INPUT_VALUE_6, 2, A55_STL_BM36_IMM_VALUE_3, A55_STL_BM36_GOLDEN_VALUE_3);
    TEST_EON_SHIFT(A55_STL_BM36_INPUT_VALUE_7, A55_STL_BM36_INPUT_VALUE_8, 3, A55_STL_BM36_IMM_VALUE_4, A55_STL_BM36_GOLDEN_VALUE_4);

    // EOR (shift)
    TEST_EOR_SHIFT(A55_STL_BM37_INPUT_VALUE_1, A55_STL_BM37_INPUT_VALUE_2, 0, A55_STL_BM37_IMM_VALUE_1, A55_STL_BM37_GOLDEN_VALUE_1);
    TEST_EOR_SHIFT(A55_STL_BM37_INPUT_VALUE_3, A55_STL_BM37_INPUT_VALUE_4, 1, A55_STL_BM37_IMM_VALUE_2, A55_STL_BM37_GOLDEN_VALUE_2);
    TEST_EOR_SHIFT(A55_STL_BM37_INPUT_VALUE_5, A55_STL_BM37_INPUT_VALUE_6, 2, A55_STL_BM37_IMM_VALUE_3, A55_STL_BM37_GOLDEN_VALUE_3);
    TEST_EOR_SHIFT(A55_STL_BM37_INPUT_VALUE_7, A55_STL_BM37_INPUT_VALUE_8, 3, A55_STL_BM37_IMM_VALUE_4, A55_STL_BM37_GOLDEN_VALUE_4);

    // ORR (shift)
    TEST_ORR_SHIFT(A55_STL_BM38_INPUT_VALUE_1, A55_STL_BM38_INPUT_VALUE_2, 2, A55_STL_BM38_IMM_VALUE_1, A55_STL_BM38_GOLDEN_VALUE_1);
    TEST_ORR_SHIFT(A55_STL_BM38_INPUT_VALUE_3, A55_STL_BM38_INPUT_VALUE_4, 1, A55_STL_BM38_IMM_VALUE_2, A55_STL_BM38_GOLDEN_VALUE_2);
    TEST_ORR_SHIFT(0, A55_STL_BM38_INPUT_VALUE_5, 2, 0, A55_STL_BM38_GOLDEN_VALUE_3); // MOV alias
    TEST_ORR_SHIFT(0, A55_STL_BM38_INPUT_VALUE_6, 2, 0, A55_STL_BM38_GOLDEN_VALUE_4); // MOV alias

    // ORN (shift)
    TEST_ORN_SHIFT(A55_STL_BM39_INPUT_VALUE_1, A55_STL_BM39_INPUT_VALUE_2, 2, A55_STL_BM39_IMM_VALUE_1, A55_STL_BM39_GOLDEN_VALUE_1);
    TEST_ORN_SHIFT(A55_STL_BM39_INPUT_VALUE_3, A55_STL_BM39_INPUT_VALUE_4, 1, A55_STL_BM39_IMM_VALUE_2, A55_STL_BM39_GOLDEN_VALUE_2);
    TEST_ORN_SHIFT(0, A55_STL_BM39_INPUT_VALUE_5, 2, 0, A55_STL_BM39_GOLDEN_VALUE_3); // MVN alias
    TEST_ORN_SHIFT(0, A55_STL_BM39_INPUT_VALUE_6, 2, 0, A55_STL_BM39_GOLDEN_VALUE_4); // MVN alias

    return 0;
}

void a55_stl_core_diagnose_p007(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p007_n001_c_impl() != 0) {
        pr_info("A55 STL Core P007: Test failed\n");
    } else {
        pr_info("A55 STL Core P007: Test passed\n");
    }
}
