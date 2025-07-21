/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of addition instructions (immediate, shifted register, extended register)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P003: " fmt

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

// ADD (immediate)
#define TEST_ADD_IMM(input, imm, golden) \
    do { \
        uint64_t result; \
        asm volatile( \
            "add %[res], %[in], %c[imm_val]\n" \
            : [res] "=r" (result) \
            : [in] "r" (input), [imm_val] "i" (imm) \
        ); \
        if (result != golden) { \
            pr_err("[%d] ADD_IMM: in=0x%llx, imm=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(input), (uint64_t)(imm), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

#define TEST_ADD_IMM_SEQ(input, imm_arr, arr_len, golden) \
    do { \
        uint64_t result = (input); \
        for (int i = 0; i < (arr_len); ++i) { \
            asm volatile( \
                "add %[res], %[in], %c[imm_val]\n" \
                : [res] "=r" (result) \
                : [in] "r" (result), [imm_val] "i" ((imm_arr)[i]) \
            ); \
        } \
        if (result != (golden)) { \
            pr_err("[%d] ADD_IMM_SEQ: in=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(input), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

// ADDS (immediate) + NZCV
#define TEST_ADDS_IMM(input, imm, golden_res, golden_flags) \
    do { \
        uint64_t result, flags; \
        asm volatile( \
            "msr nzcv, xzr\n" \
            "adds %[res], %[in], %c[imm_val]\n" \
            "mrs %[flg], nzcv\n" \
            : [res] "=&r" (result), [flg] "=&r" (flags) \
            : [in] "r" (input), [imm_val] "i" (imm) \
            : "cc" \
        ); \
        if ((result != golden_res) && ((uint32_t)flags != (uint32_t)golden_flags)) { \
            pr_err("[%d] ADDS_IMM: in=0x%llx, imm=0x%llx, got=0x%llx/0x%08x, expect=0x%llx/0x%08x\n", \
                __LINE__, (uint64_t)(input), (uint64_t)(imm), \
                (uint64_t)(result), (uint32_t)(flags), \
                (uint64_t)(golden_res), (uint32_t)(golden_flags)); \
            return -1; \
        } \
    } while (0)

// ADD (register)
#define TEST_ADD_REG(input1, input2, golden) \
    do { \
        uint64_t result; \
        asm volatile( \
            "add %[res], %[in1], %[in2]\n" \
            : [res] "=r" (result) \
            : [in1] "r" (input1), [in2] "r" (input2) \
        ); \
        if (result != golden) { \
            pr_err("[%d] ADD_REG: in1=0x%llx, in2=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, (uint64_t)(input1), (uint64_t)(input2), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

// ADDS (register, shifted)
#define TEST_ADDS_REG_SHIFT(input1, input2, shift_val, golden_res, golden_flags) \
    do { \
        uint64_t result, flags; \
        asm volatile( \
            "msr nzcv, xzr\n" \
            "adds %[res], %[in1], %[in2], lsl %[shift]\n" \
            "mrs %[flg], nzcv\n" \
            : [res] "=&r" (result), [flg] "=&r" (flags) \
            : [in1] "r" (input1), [in2] "r" (input2), [shift] "I" (shift_val) \
            : "cc" \
        ); \
        if ((result != golden_res) && ((uint32_t)flags != (uint32_t)golden_flags)) { \
            pr_err("[%d] ADDS_REG_SHIFT: in1=0x%llx, in2=0x%llx, sh=%d, got=0x%llx/0x%08x, expect=0x%llx/0x%08x\n", \
                __LINE__, (uint64_t)(input1), (uint64_t)(input2), (int)(shift_val), \
                (uint64_t)(result), (uint32_t)(flags), \
                (uint64_t)(golden_res), (uint32_t)(golden_flags)); \
            return -1; \
        } \
    } while (0)

static inline int test_add_ext(uint64_t input1, uint64_t input2, int ext_type, int ext_amt, uint64_t golden) {
    uint64_t ext_val = 0;
    switch (ext_type) {
        case 0: ext_val = (uint32_t)(input2) & 0xFF; break;
        case 1: ext_val = (uint32_t)(input2) & 0xFFFF; break;
        case 2: ext_val = (uint32_t)(input2) & 0xFFFFFFFF; break;
        default: ext_val = (uint64_t)(input2); break;
    }
    ext_val <<= ext_amt;
    uint64_t result;
    asm volatile("add %[res], %[in1], %[extv]\n"
        : [res] "=r" (result)
        : [in1] "r" (input1), [extv] "r" (ext_val));
    if (result != golden) {
        pr_err("[%d] ADD_EXT: in1=0x%llx, in2=0x%llx, ext=%d, amt=%d, got=0x%llx, expect=0x%llx\n",
            __LINE__, (uint64_t)(input1), (uint64_t)(input2), ext_type, ext_amt, result, golden);
        return -1;
    }
    return 0;
}

static inline int test_adds_ext(uint64_t input1, uint64_t input2, int ext_type, int ext_amt, uint64_t golden_res, uint32_t golden_flags) {
    uint64_t ext_val = 0;
    switch (ext_type) {
        case 0: ext_val = (uint32_t)(input2) & 0xFF; break;
        case 1: ext_val = (uint32_t)(input2) & 0xFFFF; break;
        case 2: ext_val = (uint32_t)(input2) & 0xFFFFFFFF; break;
        default: ext_val = (uint64_t)(input2); break;
    }
    ext_val <<= ext_amt;
    uint64_t result, flags;
    asm volatile("msr nzcv, xzr\n"
        "adds %[res], %[in1], %[extv]\n"
        "mrs %[flg], nzcv\n"
        : [res] "=&r" (result), [flg] "=&r" (flags)
        : [in1] "r" (input1), [extv] "r" (ext_val)
        : "cc");
    if ((result != golden_res) && ((uint32_t)flags != golden_flags)) {
        pr_err("[%d] ADDS_EXT: in1=0x%llx, in2=0x%llx, ext=%d, amt=%d, got=0x%llx/0x%08x, expect=0x%llx/0x%08x\n",
            __LINE__, (uint64_t)(input1), (uint64_t)(input2), ext_type, ext_amt,
            result, (uint32_t)flags, golden_res, golden_flags);
        return -1;
    }
    return 0;
}

#define TEST_ADDS_EXT(input1, input2, ext_type, ext_amt, golden_res, golden_flags) \
    do { \
        if (test_adds_ext((input1), (input2), (ext_type), (ext_amt), (golden_res), (golden_flags))) \
            return -1; \
    } while (0)

#define TEST_ADD_EXT(input1, input2, ext_type, ext_amt, golden) \
    do { \
        if (test_add_ext((input1), (input2), (ext_type), (ext_amt), (golden))) \
            return -1; \
    } while (0)

int a55_stl_core_diagnose_p003_n001_c_impl(void) {
	// Basic Module 07: ADD (immediate)
    static const uint64_t bm07_imms[] = {
        A55_STL_BM07_IMM_VALUE_1,
        A55_STL_BM07_IMM_VALUE_2,
        A55_STL_BM07_IMM_VALUE_3,
        A55_STL_BM07_IMM_VALUE_4
    };
    TEST_ADD_IMM_SEQ(A55_STL_BM07_INPUT_VALUE_1, bm07_imms, 4, A55_STL_BM07_GOLDEN_VALUE_1);
    TEST_ADD_IMM_SEQ(A55_STL_BM07_INPUT_VALUE_2, bm07_imms, 4, A55_STL_BM07_GOLDEN_VALUE_2);

    // Basic Module 08: ADDS (immediate)
	TEST_ADDS_IMM(A55_STL_BM08_INPUT_VALUE_1, A55_STL_BM08_IMM_VALUE_1, A55_STL_BM08_GOLDEN_VALUE_2, A55_STL_BM08_GOLDEN_FLAGS_1);
	TEST_ADDS_IMM(A55_STL_BM08_INPUT_VALUE_2, A55_STL_BM08_IMM_VALUE_1, A55_STL_BM08_GOLDEN_VALUE_4, A55_STL_BM08_GOLDEN_FLAGS_2);

    TEST_ADDS_IMM(A55_STL_BM08_INPUT_VALUE_3, A55_STL_BM08_IMM_VALUE_2, A55_STL_BM08_GOLDEN_VALUE_1, A55_STL_BM08_GOLDEN_FLAGS_1);
    TEST_ADDS_IMM(A55_STL_BM08_INPUT_VALUE_1, A55_STL_BM08_IMM_VALUE_1, A55_STL_BM08_GOLDEN_VALUE_2, A55_STL_BM08_GOLDEN_FLAGS_1);
    TEST_ADDS_IMM(A55_STL_BM08_INPUT_VALUE_4, A55_STL_BM08_IMM_VALUE_2, A55_STL_BM08_GOLDEN_VALUE_3, A55_STL_BM08_GOLDEN_FLAGS_2);
    TEST_ADDS_IMM(A55_STL_BM08_INPUT_VALUE_2, A55_STL_BM08_IMM_VALUE_1, A55_STL_BM08_GOLDEN_VALUE_4, A55_STL_BM08_GOLDEN_FLAGS_2);

    // Basic Module 20: ADD (register)
    TEST_ADD_REG(A55_STL_BM20_INPUT_VALUE_1, A55_STL_BM20_INPUT_VALUE_3, A55_STL_BM20_GOLDEN_VALUE_1);
    TEST_ADD_REG(A55_STL_BM20_INPUT_VALUE_2, A55_STL_BM20_INPUT_VALUE_4, A55_STL_BM20_GOLDEN_VALUE_2);
    TEST_ADD_REG(A55_STL_BM20_INPUT_VALUE_5, A55_STL_BM20_INPUT_VALUE_7, A55_STL_BM20_GOLDEN_VALUE_3);
    TEST_ADD_REG(A55_STL_BM20_INPUT_VALUE_6, A55_STL_BM20_INPUT_VALUE_8, A55_STL_BM20_GOLDEN_VALUE_4);

    // Basic Module 21: ADDS (register, shifted)
    TEST_ADDS_REG_SHIFT(A55_STL_BM21_INPUT_VALUE_3, A55_STL_BM21_INPUT_VALUE_4, A55_STL_BM21_IMM_VALUE_1, A55_STL_BM21_GOLDEN_VALUE_1, A55_STL_BM21_GOLDEN_FLAGS_1);

    // Basic Module 24: ADD (extended register)
    TEST_ADD_EXT(A55_STL_BM24_INPUT_VALUE_1, A55_STL_BM24_INPUT_VALUE_2, 0, A55_STL_BM24_IMM_VALUE_1, A55_STL_BM24_GOLDEN_VALUE_1);
    TEST_ADD_EXT(A55_STL_BM24_INPUT_VALUE_3, A55_STL_BM24_INPUT_VALUE_4, 1, A55_STL_BM24_IMM_VALUE_2, A55_STL_BM24_GOLDEN_VALUE_2);
    TEST_ADD_EXT(A55_STL_BM24_INPUT_VALUE_5, A55_STL_BM24_INPUT_VALUE_6, 2, A55_STL_BM24_IMM_VALUE_1, A55_STL_BM24_GOLDEN_VALUE_3);
    TEST_ADD_EXT(A55_STL_BM24_INPUT_VALUE_7, A55_STL_BM24_INPUT_VALUE_8, 3, A55_STL_BM24_IMM_VALUE_1, A55_STL_BM24_GOLDEN_VALUE_4);

    // Basic Module 25: ADDS (extended register)
    TEST_ADDS_EXT(A55_STL_BM25_INPUT_VALUE_3, A55_STL_BM25_INPUT_VALUE_4, 3, A55_STL_BM25_IMM_VALUE_1, A55_STL_BM25_GOLDEN_VALUE_1, A55_STL_BM25_GOLDEN_FLAGS_1);
    TEST_ADDS_EXT(A55_STL_BM25_INPUT_VALUE_5, A55_STL_BM25_INPUT_VALUE_4, 3, A55_STL_BM25_IMM_VALUE_2, A55_STL_BM25_GOLDEN_VALUE_3, A55_STL_BM25_GOLDEN_FLAGS_2);
    TEST_ADDS_EXT(A55_STL_BM25_INPUT_VALUE_6, A55_STL_BM25_INPUT_VALUE_4, 1, A55_STL_BM25_IMM_VALUE_3, A55_STL_BM25_GOLDEN_VALUE_5, A55_STL_BM25_GOLDEN_FLAGS_3);

    return 0;
}

void a55_stl_core_diagnose_p003(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p003_n001_c_impl() != 0) {
        pr_info("A55 STL Core P003: Test failed\n");
    } else {
        pr_info("A55 STL Core P003: Test passed\n");
    }
}
