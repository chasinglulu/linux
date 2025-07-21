/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of bitfield and extract instructions
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P002: " fmt

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

#define TEST_BFM(dest_val, src_val, immr, imms, golden) \
    do { \
        uint64_t result = dest_val; \
        asm volatile( \
            "bfm %[res], %[src], %[r], %[s]\n" \
            : [res] "+r" (result) \
            : [src] "r" (src_val), [r] "I" (immr), [s] "I" (imms) \
        ); \
        if (result != golden) { \
            pr_err("[%d] BFM: dest=0x%llx, src=0x%llx, immr=%d, imms=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(dest_val), (uint64_t)(src_val), (int)(immr), (int)(imms), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

#define TEST_SBFM(src_val, immr, imms, golden) \
    do { \
        uint64_t result; \
        asm volatile( \
            "sbfm %[res], %[src], %[r], %[s]\n" \
            : [res] "=r" (result) \
            : [src] "r" (src_val), [r] "I" (immr), [s] "I" (imms) \
        ); \
        if (result != golden) { \
            pr_err("[%d] SBFM: src=0x%llx, immr=%d, imms=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(src_val), (int)(immr), (int)(imms), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

#define TEST_UBFM_X(src_val, immr, imms, golden) \
    do { \
        uint64_t result; \
        asm volatile( \
            "ubfm %[res], %[src], %[r], %[s]\n" \
            : [res] "=r" (result) \
            : [src] "r" (src_val), [r] "I" (immr), [s] "I" (imms) \
        ); \
        if (result != golden) { \
            pr_err("[%d] UBFM_X: src=0x%llx, immr=%d, imms=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(src_val), (int)(immr), (int)(imms), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

#define TEST_UBFM_W(src_val, immr, imms, golden) \
    do { \
        uint32_t result; \
        asm volatile( \
            "ubfm %w[res], %w[src], %[r], %[s]\n" \
            : [res] "=r" (result) \
            : [src] "r" ((uint32_t)src_val), [r] "I" (immr), [s] "I" (imms) \
        ); \
        if (result != (uint32_t)golden) { \
            pr_err("[%d] UBFM_W: src=0x%x, immr=%d, imms=%d, got=0x%x, expect=0x%x\n", \
                __LINE__, (uint32_t)(src_val), (int)(immr), (int)(imms), (uint32_t)(result), (uint32_t)(golden)); \
            return -1; \
        } \
    } while (0)

#define TEST_EXTR(val_n, val_m, lsb, golden) \
    do { \
        uint64_t result; \
        asm volatile( \
            "extr %[res], %[vn], %[vm], %[lsb_val]\n" \
            : [res] "=r" (result) \
            : [vn] "r" (val_n), [vm] "r" (val_m), [lsb_val] "I" (lsb) \
        ); \
        if (result != golden) { \
            pr_err("[%d] EXTR: vn=0x%llx, vm=0x%llx, lsb=%d, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(val_n), (uint64_t)(val_m), (int)(lsb), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)


int a55_stl_core_diagnose_p002_n001_c_impl(void) {
    // Basic Module 16: BFM
    TEST_BFM(A55_STL_BM16_IMM_VALUE_1, A55_STL_BM16_INPUT_VALUE_1, A55_STL_BM16_IMM_VALUE_1, A55_STL_BM16_IMM_VALUE_2, A55_STL_BM16_GOLDEN_VALUE_1);
    TEST_BFM(A55_STL_BM16_INPUT_VALUE_2, A55_STL_BM16_IMM_VALUE_1, A55_STL_BM16_IMM_VALUE_3, A55_STL_BM16_IMM_VALUE_2, A55_STL_BM16_GOLDEN_VALUE_2);
    TEST_BFM(A55_STL_BM16_IMM_VALUE_1, A55_STL_BM16_INPUT_VALUE_3, A55_STL_BM16_IMM_VALUE_4, A55_STL_BM16_IMM_VALUE_5, A55_STL_BM16_GOLDEN_VALUE_3);
    TEST_BFM(A55_STL_BM16_IMM_VALUE_1, A55_STL_BM16_INPUT_VALUE_4, A55_STL_BM16_IMM_VALUE_4, A55_STL_BM16_IMM_VALUE_5, A55_STL_BM16_GOLDEN_VALUE_4);

    // Basic Module 17: SBFM
    TEST_SBFM(A55_STL_BM17_INPUT_VALUE_1, A55_STL_BM17_IMM_VALUE_2, A55_STL_BM17_IMM_VALUE_4, A55_STL_BM17_GOLDEN_VALUE_1); // SBFX
    TEST_SBFM(A55_STL_BM17_INPUT_VALUE_2, A55_STL_BM17_IMM_VALUE_3, A55_STL_BM17_IMM_VALUE_2, A55_STL_BM17_GOLDEN_VALUE_2); // SBFIZ
    TEST_SBFM(A55_STL_BM17_INPUT_VALUE_3, A55_STL_BM17_IMM_VALUE_1, A55_STL_BM17_IMM_VALUE_4, A55_STL_BM17_GOLDEN_VALUE_3); // SXTB
    TEST_SBFM(A55_STL_BM17_INPUT_VALUE_4, A55_STL_BM17_IMM_VALUE_1, A55_STL_BM17_IMM_VALUE_5, A55_STL_BM17_GOLDEN_VALUE_4); // SXTH
    TEST_SBFM(A55_STL_BM17_INPUT_VALUE_5, A55_STL_BM17_IMM_VALUE_1, A55_STL_BM17_IMM_VALUE_6, A55_STL_BM17_GOLDEN_VALUE_5); // SXTW
    TEST_SBFM(A55_STL_BM17_INPUT_VALUE_6, A55_STL_BM17_IMM_VALUE_7, A55_STL_BM17_IMM_VALUE_8, A55_STL_BM17_GOLDEN_VALUE_6); // ASR

    // Basic Module 18: UBFM
    TEST_UBFM_W(A55_STL_BM18_INPUT_VALUE_1, A55_STL_BM18_IMM_VALUE_1, A55_STL_BM18_IMM_VALUE_2, A55_STL_BM18_GOLDEN_VALUE_1); // UXTB
    TEST_UBFM_W(A55_STL_BM18_INPUT_VALUE_2, A55_STL_BM18_IMM_VALUE_1, A55_STL_BM18_IMM_VALUE_3, A55_STL_BM18_GOLDEN_VALUE_2); // UXTH
    TEST_UBFM_X(A55_STL_BM18_INPUT_VALUE_3, A55_STL_BM18_IMM_VALUE_4, A55_STL_BM18_IMM_VALUE_5, A55_STL_BM18_GOLDEN_VALUE_3); // LSL
    TEST_UBFM_X(A55_STL_BM18_INPUT_VALUE_4, A55_STL_BM18_IMM_VALUE_6, A55_STL_BM18_IMM_VALUE_7, A55_STL_BM18_GOLDEN_VALUE_4); // LSR
    TEST_UBFM_X(A55_STL_BM18_INPUT_VALUE_5, A55_STL_BM18_IMM_VALUE_8, A55_STL_BM18_IMM_VALUE_9, A55_STL_BM18_GOLDEN_VALUE_5); // UBFIZ
    TEST_UBFM_X(A55_STL_BM18_INPUT_VALUE_6, A55_STL_BM18_IMM_VALUE_1, A55_STL_BM18_IMM_VALUE_3, A55_STL_BM18_GOLDEN_VALUE_6); // UBFX

    // Basic Module 19: EXTR
    TEST_EXTR(A55_STL_BM19_INPUT_VALUE_1, A55_STL_BM19_INPUT_VALUE_2, A55_STL_BM19_IMM_VALUE_1, A55_STL_BM19_GOLDEN_VALUE_1);
    TEST_EXTR(A55_STL_BM19_INPUT_VALUE_3, A55_STL_BM19_INPUT_VALUE_4, A55_STL_BM19_IMM_VALUE_2, A55_STL_BM19_GOLDEN_VALUE_2);
    TEST_EXTR(A55_STL_BM19_INPUT_VALUE_5, A55_STL_BM19_INPUT_VALUE_6, A55_STL_BM19_IMM_VALUE_3, A55_STL_BM19_GOLDEN_VALUE_3);
    TEST_EXTR(A55_STL_BM19_INPUT_VALUE_7, A55_STL_BM19_INPUT_VALUE_8, A55_STL_BM19_IMM_VALUE_4, A55_STL_BM19_GOLDEN_VALUE_4);

    // All tests passed
    return 0;
}

void a55_stl_core_diagnose_p002(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p002_n001_c_impl() != 0) {
        pr_info("A55 STL Core P002: Test failed\n");
    } else {
        pr_info("A55 STL Core P002: Test passed\n");
    }
}