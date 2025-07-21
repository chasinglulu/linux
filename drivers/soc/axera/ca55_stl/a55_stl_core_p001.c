/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of logical instructions AND (both immediate and shifted register),
 * ANDS (both immediate and shifted register), EOR and ORR
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P001: " fmt

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

// External assembly function declarations
// extern void a55_stl_preamble(void);
// extern void a55_stl_postamble(void);
// extern void a55_stl_set_fctlr_ping(void);
// extern void a55_stl_set_ffmir_exception(void);
// extern void a55_stl_set_regs_error(void);

// Macro: Test logic instruction (immediate), does not check flags
#define TEST_LOGIC_IMM(op, input, imm, golden) \
    do { \
        uint64_t result; \
        asm volatile( \
            #op " %[res], %[in], %c[imm_val]\n" \
            : [res] "=r" (result) \
            : [in] "r" (input), [imm_val] "i" (imm) \
        ); \
        if (result != golden) { \
            pr_err("[%d] IMM: %s in=0x%llx, imm=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, #op, (uint64_t)(input), (uint64_t)(imm), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

#define TEST_LOGIC_REG(op, input1, input2, golden) \
    do { \
        uint64_t result; \
        asm volatile( \
            #op " %[res], %[in1], %[in2]\n" \
            : [res] "=r" (result) \
            : [in1] "r" (input1), [in2] "r" (input2) \
        ); \
        if (result != golden) { \
            pr_err("[%d] REG: %s in1=0x%llx, in2=0x%llx, got=0x%llx, expect=0x%llx\n", \
                __LINE__, #op, (uint64_t)(input1), (uint64_t)(input2), (uint64_t)(result), (uint64_t)(golden)); \
            return -1; \
        } \
    } while (0)

#define TEST_LOGICS_IMM(input, imm, golden_res, golden_flags) \
    do { \
        uint64_t result; \
        uint64_t flags; \
        asm volatile( \
            "msr nzcv, xzr\n" \
            "ands %[res], %[in], %c[imm_val]\n" \
            "mrs %[flg], nzcv\n" \
            : [res] "=&r" (result), [flg] "=&r" (flags) \
            : [in] "r" (input), [imm_val] "i" (imm) \
            : "cc" \
        ); \
        if ((result != golden_res) && ((uint32_t)flags != (uint32_t)golden_flags)) { \
            pr_err("[%d] IMM+FLAGS: ands in=0x%llx, imm=0x%llx, got=0x%llx/0x%08x, expect=0x%llx/0x%08x\n", \
                __LINE__, (uint64_t)(input), (uint64_t)(imm), \
                (uint64_t)(result), (uint32_t)(flags), \
                (uint64_t)(golden_res), (uint32_t)(golden_flags)); \
            return -1; \
        } \
    } while (0)

#define TEST_LOGICS_REG_SHIFT(input1, input2, shift_val, golden_res, golden_flags) \
    do { \
        uint64_t result; \
        uint64_t flags; \
        asm volatile( \
            "msr nzcv, xzr\n" \
            "ands %[res], %[in1], %[in2], lsl %[shift]\n" \
            "mrs %[flg], nzcv\n" \
            : [res] "=&r" (result), [flg] "=&r" (flags) \
            : [in1] "r" (input1), [in2] "r" (input2), [shift] "I" (shift_val) \
            : "cc" \
        ); \
        if ((result != golden_res) && ((uint32_t)flags != (uint32_t)golden_flags)) { \
            pr_err("[%d] REG+SHIFT: ands in1=0x%llx, in2=0x%llx, sh=%d, got=0x%llx/0x%08x, expect=0x%llx/0x%08x\n", \
                __LINE__, (uint64_t)(input1), (uint64_t)(input2), (int)(shift_val), \
                (uint64_t)(result), (uint32_t)(flags), \
                (uint64_t)(golden_res), (uint32_t)(golden_flags)); \
            return -1; \
        } \
    } while (0)

int a55_stl_core_diagnose_p001_n001_c_impl(void) {
    // Basic Module 03: AND (immediate)
    TEST_LOGIC_IMM(and, A55_STL_BM03_INPUT_VALUE_1, A55_STL_BM03_IMM_VALUE_1, A55_STL_BM03_GOLDEN_VALUE_1);
    TEST_LOGIC_IMM(and, A55_STL_BM03_INPUT_VALUE_1, A55_STL_BM03_IMM_VALUE_2, A55_STL_BM03_GOLDEN_VALUE_2);
    TEST_LOGIC_IMM(and, A55_STL_BM03_INPUT_VALUE_2, A55_STL_BM03_IMM_VALUE_1, A55_STL_BM03_GOLDEN_VALUE_3);
    TEST_LOGIC_IMM(and, A55_STL_BM03_INPUT_VALUE_2, A55_STL_BM03_IMM_VALUE_2, A55_STL_BM03_GOLDEN_VALUE_4);

    // Basic Module 04: ANDS (immediate)
    // Test N = 1, Z = 0, C = 0, V = 0
    TEST_LOGICS_IMM(A55_STL_BM04_INPUT_VALUE_1, A55_STL_BM04_IMM_VALUE_1, A55_STL_BM04_GOLDEN_VALUE_1, A55_STL_BM04_GOLDEN_FLAGS_1);
	// Test N = 0, Z = 1, C = 0, V = 0
    TEST_LOGICS_IMM(A55_STL_BM04_INPUT_VALUE_2, A55_STL_BM04_GOLDEN_VALUE_3, A55_STL_BM04_GOLDEN_VALUE_4, A55_STL_BM04_GOLDEN_FLAGS_2);

    TEST_LOGICS_IMM(A55_STL_BM04_INPUT_VALUE_1, A55_STL_BM04_IMM_VALUE_1, A55_STL_BM04_GOLDEN_VALUE_1, A55_STL_BM04_GOLDEN_FLAGS_1);
    TEST_LOGICS_IMM(A55_STL_BM04_INPUT_VALUE_2, A55_STL_BM04_IMM_VALUE_2, A55_STL_BM04_GOLDEN_VALUE_2, A55_STL_BM04_GOLDEN_FLAGS_1);
    TEST_LOGICS_IMM(A55_STL_BM04_INPUT_VALUE_3, A55_STL_BM04_IMM_VALUE_3, A55_STL_BM04_GOLDEN_VALUE_3, A55_STL_BM04_GOLDEN_FLAGS_2);
    TEST_LOGICS_IMM(A55_STL_BM04_INPUT_VALUE_4, A55_STL_BM04_IMM_VALUE_4, A55_STL_BM04_GOLDEN_VALUE_4, A55_STL_BM04_GOLDEN_FLAGS_2);

    // Basic Module 05: EOR (immediate)
    TEST_LOGIC_IMM(eor, A55_STL_BM05_INPUT_VALUE_1, A55_STL_BM05_IMM_VALUE_1, A55_STL_BM05_GOLDEN_VALUE_1);
    TEST_LOGIC_IMM(eor, A55_STL_BM05_INPUT_VALUE_2, A55_STL_BM05_IMM_VALUE_2, A55_STL_BM05_GOLDEN_VALUE_2);
    TEST_LOGIC_IMM(eor, A55_STL_BM05_INPUT_VALUE_3, A55_STL_BM05_IMM_VALUE_3, A55_STL_BM05_GOLDEN_VALUE_3);
    TEST_LOGIC_IMM(eor, A55_STL_BM05_INPUT_VALUE_4, A55_STL_BM05_IMM_VALUE_4, A55_STL_BM05_GOLDEN_VALUE_4);

    // Basic Module 06: ORR (immediate)
    TEST_LOGIC_IMM(orr, A55_STL_BM06_INPUT_VALUE_1, A55_STL_BM06_IMM_VALUE_1, A55_STL_BM06_GOLDEN_VALUE_1);
    TEST_LOGIC_IMM(orr, A55_STL_BM06_INPUT_VALUE_2, A55_STL_BM06_IMM_VALUE_2, A55_STL_BM06_GOLDEN_VALUE_2);
    TEST_LOGIC_IMM(orr, A55_STL_BM06_INPUT_VALUE_3, A55_STL_BM06_IMM_VALUE_3, A55_STL_BM06_GOLDEN_VALUE_3);
    TEST_LOGIC_IMM(orr, A55_STL_BM06_INPUT_VALUE_4, A55_STL_BM06_IMM_VALUE_4, A55_STL_BM06_GOLDEN_VALUE_4);

    // Basic Module 32: AND (register)
    TEST_LOGIC_REG(and, A55_STL_BM32_INPUT_VALUE_1, A55_STL_BM32_INPUT_VALUE_3, A55_STL_BM32_GOLDEN_VALUE_1);
    TEST_LOGIC_REG(and, A55_STL_BM32_INPUT_VALUE_2, A55_STL_BM32_INPUT_VALUE_4, A55_STL_BM32_GOLDEN_VALUE_2);
    TEST_LOGIC_REG(and, A55_STL_BM32_INPUT_VALUE_5, A55_STL_BM32_INPUT_VALUE_7, A55_STL_BM32_GOLDEN_VALUE_3);
    TEST_LOGIC_REG(and, A55_STL_BM32_INPUT_VALUE_6, A55_STL_BM32_INPUT_VALUE_8, A55_STL_BM32_GOLDEN_VALUE_4);

    // Basic Module 33: ANDS (register, shifted)
    TEST_LOGICS_REG_SHIFT(A55_STL_BM33_INPUT_VALUE_3, A55_STL_BM33_INPUT_VALUE_4, A55_STL_BM33_IMM_VALUE_1, A55_STL_BM33_GOLDEN_VALUE_1, A55_STL_BM33_GOLDEN_FLAGS_1);
    TEST_LOGICS_REG_SHIFT(A55_STL_BM33_INPUT_VALUE_5, A55_STL_BM33_INPUT_VALUE_6, A55_STL_BM33_IMM_VALUE_1, A55_STL_BM33_GOLDEN_VALUE_1, A55_STL_BM33_GOLDEN_FLAGS_2);

    TEST_LOGICS_REG_SHIFT(A55_STL_BM33_INPUT_VALUE_1, A55_STL_BM33_INPUT_VALUE_2, A55_STL_BM33_IMM_VALUE_2, A55_STL_BM33_GOLDEN_VALUE_1, A55_STL_BM33_GOLDEN_FLAGS_1);
    TEST_LOGICS_REG_SHIFT(A55_STL_BM33_INPUT_VALUE_1, A55_STL_BM33_INPUT_VALUE_2, A55_STL_BM33_IMM_VALUE_2, A55_STL_BM33_GOLDEN_VALUE_2, A55_STL_BM33_GOLDEN_FLAGS_2);
    TEST_LOGICS_REG_SHIFT(A55_STL_BM33_INPUT_VALUE_7, A55_STL_BM33_INPUT_VALUE_4, A55_STL_BM33_IMM_VALUE_2, A55_STL_BM33_GOLDEN_VALUE_3, A55_STL_BM33_GOLDEN_FLAGS_1);

    // All tests passed
    return 0;
}

// C language entry function, keep the same signature as the original assembly function
void a55_stl_core_diagnose_p001(a55_stl_state_t *fctlr_base) {
    // Parameters x0 and x1 are managed by the caller in the C environment,
    // and it is assumed here that they are set properly,
    // and that a55_stl_preamble and other functions can access them correctly.

    // a55_stl_preamble();
    // a55_stl_set_fctlr_ping();

	//a55_stl_set_ffmir_exception();
    if (a55_stl_core_diagnose_p001_n001_c_impl() != 0) {
        // Test failed
        // a55_stl_set_regs_error();
		pr_info("A55 STL Core P001: Test failed\n");
    } else {
        // Test passed, set PDONE status
        // Need to implement a55_stl_set_fctlr_pdone() here
        // mov x2, A55_STL_FCTLR_STATUS_PDONE
		pr_info("A55 STL Core P001: Test passed\n");
    }

    // a55_stl_postamble();
}