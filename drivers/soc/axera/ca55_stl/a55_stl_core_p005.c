/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * Testing of branch and address instructions (B.cond, BLR, BR, CBZ, TBZ, CBNZ, TBNZ, ADR)
 *
 * Copyright (C) 2025 Charleye <wangkart@aliyun.com>
 */

#define pr_fmt(fmt) "A55-STL-Core-P005: " fmt

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

// BM67: B.cond test
static int test_bm67(void) {
    uint64_t signature = A55_STL_SIGN_SEED;
    uint64_t golden = A55_STL_BM67_GOLDEN_SIGN;
    int fail = 0;

    asm volatile(
        "mov x29, %[sign]\n"
        "mov x10, %[init0]\n"
        "mov x8, %[init0]\n"
        "mov x11, %[in1]\n"
        "mov x12, %[in2]\n"
        // EQ
        "mov x10, %[in3]\n"
        "mov x9, %[jmp1_flags]\n"
        "msr nzcv, x9\n"
        "b.eq 103f\n"
        "eor x10, x11, x10\n"
        "100:\n"
        // NE
        "mvn x10, x12, lsr %[jmp2_lsr3]\n"
        "msr nzcv, x8\n"
        "b.ne 111f\n"
        "eor x10, x12, x10\n"
        "101:\n"
        // CS/HS
        "add x10, x10, x11, lsl %[jmp3_lsl1]\n"
        "mov x9, %[jmp3_flags]\n"
        "msr nzcv, x9\n"
        "b.cs 107f\n"
        "eor x10, x11, x10\n"
        "102:\n"
        // CC/LO
        "sub x10, x10, x12, lsl %[jmp4_lsl2]\n"
        "msr nzcv, x8\n"
        "b.cc 109f\n"
        "eor x10, x12, x10\n"
        "103:\n"
        // MI
        "add x10, x10, x11, lsr %[jmp5_lsr1]\n"
        "mov x9, %[jmp5_flags]\n"
        "msr nzcv, x9\n"
        "b.mi 110f\n"
        "eor x10, x11, x10\n"
        "104:\n"
        // PL
        "add x10, x10, x12, lsl %[jmp6_lsl4]\n"
        "msr nzcv, x8\n"
        "b.pl 106f\n"
        "eor x10, x12, x10\n"
        "105:\n"
        // VS
        "sub x10, x10, x11, lsl %[jmp7_lsl1]\n"
        "mov x9, %[jmp7_flags]\n"
        "msr nzcv, x9\n"
        "b.vs 100b\n"
        "eor x10, x11, x10\n"
        "106:\n"
        // VC
        "sub x10, x10, x11, lsr %[jmp8_lsr3]\n"
        "msr nzcv, x8\n"
        "b.vc 101b\n"
        "eor x10, x11, x10\n"
        "107:\n"
        // HI
        "add x10, x10, x12, asr %[jmp9_asr1]\n"
        "mov x9, %[jmp9_flags]\n"
        "msr nzcv, x9\n"
        "b.hi 102b\n"
        "eor x10, x12, x10\n"
        "108:\n"
        // LS
        "add x10, x10, x11, lsl %[jmp10_lsl4]\n"
        "mov x9, %[jmp10_flags]\n"
        "msr nzcv, x9\n"
        "b.ls 105b\n"
        "eor x10, x11, x10\n"
        "109:\n"
        // GE
        "sub x10, x10, x12, lsr %[jmp11_lsr3]\n"
        "mov x9, %[jmp11_flags]\n"
        "msr nzcv, x9\n"
        "b.ge 112f\n"
        "eor x10, x12, x10\n"
        "110:\n"
        // LT
        "add x10, x10, x11, lsr %[jmp12_lsr4]\n"
        "mov x9, %[jmp12_flags]\n"
        "msr nzcv, x9\n"
        "b.lt 104b\n"
        "eor x10, x11, x10\n"
        "111:\n"
        // GT
        "add x10, x10, x12, lsl %[jmp13_lsl3]\n"
        "msr nzcv, x8\n"
        "b.gt 113f\n"
        "eor x10, x12, x10\n"
        "112:\n"
        // LE
        "sub x10, x10, x12, lsl %[jmp14_lsl2]\n"
        "mov x9, %[jmp14_flags]\n"
        "msr nzcv, x9\n"
        "b.le 108b\n"
        "eor x10, x12, x10\n"
        "113:\n"
        // Signature accumulation
        "ror x29, x29, %[rot]\n"
        "eor x29, x10, x29\n"
        "mov %[sign], x29\n"
        : [sign] "+r"(signature)
        : [init0] "r"(A55_STL_BM67_INIT_0_VALUE),
          [in1] "r"(A55_STL_BM67_INPUT_VALUE_1),
          [in2] "r"(A55_STL_BM67_INPUT_VALUE_2),
          [in3] "r"(A55_STL_BM67_INPUT_VALUE_3),
          [jmp1_flags] "r"(A55_STL_BM67_JMP1_FLAGS),
          [jmp2_lsr3] "I"(A55_STL_BM67_JMP2_LSR_3),
          [jmp3_lsl1] "I"(A55_STL_BM67_JMP3_LSL_1),
          [jmp3_flags] "r"(A55_STL_BM67_JMP3_FLAGS),
          [jmp4_lsl2] "I"(A55_STL_BM67_JMP4_LSL_2),
          [jmp5_lsr1] "I"(A55_STL_BM67_JMP5_LSR_1),
          [jmp5_flags] "r"(A55_STL_BM67_JMP5_FLAGS),
          [jmp6_lsl4] "I"(A55_STL_BM67_JMP6_LSL_4),
          [jmp7_lsl1] "I"(A55_STL_BM67_JMP7_LSL_1),
          [jmp7_flags] "r"(A55_STL_BM67_JMP7_FLAGS),
          [jmp8_lsr3] "I"(A55_STL_BM67_JMP8_LSR_3),
          [jmp9_asr1] "I"(A55_STL_BM67_JMP9_ASR_1),
          [jmp9_flags] "r"(A55_STL_BM67_JMP9_FLAGS),
          [jmp10_lsl4] "I"(A55_STL_BM67_JMP10_LSL_4),
          [jmp10_flags] "r"(A55_STL_BM67_JMP10_FLAGS),
          [jmp11_lsr3] "I"(A55_STL_BM67_JMP11_LSR_3),
          [jmp11_flags] "r"(A55_STL_BM67_JMP11_FLAGS),
          [jmp12_lsr4] "I"(A55_STL_BM67_JMP12_LSR_4),
          [jmp12_flags] "r"(A55_STL_BM67_JMP12_FLAGS),
          [jmp13_lsl3] "I"(A55_STL_BM67_JMP13_LSL_3),
          [jmp14_lsl2] "I"(A55_STL_BM67_JMP14_LSL_2),
          [jmp14_flags] "r"(A55_STL_BM67_JMP14_FLAGS),
          [rot] "I"(A55_STL_ROTATE_1)
        : "x8", "x9", "x10", "x11", "x12", "x29", "memory"
    );
    if (signature != golden) {
        pr_err("BM67 signature mismatch: got=0x%llx, expect=0x%llx\n", signature, golden);
        fail = -1;
    }
    return fail;
}

// BM68: BLR, BR, CBZ, TBZ, CBNZ, TBNZ test
static int test_bm68(void) {
    uint64_t signature = A55_STL_SIGN_SEED;
    uint64_t golden = A55_STL_BM68_GOLDEN_SIGN;
    int fail = 0;

    asm volatile(
        "mov x29, %[sign]\n"
        "mov x10, %[init0]\n"
        "mov x11, %[in1]\n"
        "mov x12, %[in2]\n"
        // BLR
        "mov x10, %[in3]\n"
        "adr x9, 100f\n"
        "blr x9\n"
        "eor x10, x11, x10\n"
        // BR
        "101:\n"
        "mvn x10, x12, lsr %[jmp17_lsr3]\n"
        "adr x9, 102f\n"
        "br x9\n"
        "eor x10, x12, x10\n"
        // CBZ
        "100:\n"
        "add x10, x10, x11, lsl %[jmp18_lsl1]\n"
        "mov x9, %[init0]\n"
        "cbz x9, 104f\n"
        "eor x10, x11, x10\n"
        // TBZ
        "103:\n"
        "sub x10, x10, x12, lsl %[jmp19_lsl2]\n"
        "mov x9, %[init0]\n"
        "tbz x9, %[imm0], 101b\n"
        "eor x10, x12, x10\n"
        // CBNZ
        "102:\n"
        "add x10, x10, x11, lsr %[jmp20_lsr1]\n"
        "mov x9, %[init1]\n"
        "cbnz x9, 105f\n"
        "eor x10, x11, x10\n"
        // TBNZ
        "104:\n"
        "add x10, x10, x12, lsl %[jmp21_lsl4]\n"
        "mov x9, %[init1]\n"
        "tbnz x9, %[imm0], 103b\n"
        "eor x10, x12, x10\n"
        "105:\n"
        // Signature accumulation
        "ror x29, x29, %[rot]\n"
        "eor x29, x10, x29\n"
        "mov %[sign], x29\n"
        :
          [sign] "+r"(signature)
        : [init0] "r"(A55_STL_BM68_INIT_0_VALUE),
          [in1] "r"(A55_STL_BM68_INPUT_VALUE_1),
          [in2] "r"(A55_STL_BM68_INPUT_VALUE_2),
          [in3] "r"(A55_STL_BM68_INPUT_VALUE_3),
          [jmp17_lsr3] "I"(A55_STL_BM68_JMP17_LSR_3),
          [jmp18_lsl1] "I"(A55_STL_BM68_JMP18_LSL_1),
          [jmp19_lsl2] "I"(A55_STL_BM68_JMP19_LSL_2),
          [jmp20_lsr1] "I"(A55_STL_BM68_JMP20_LSR_1),
          [jmp21_lsl4] "I"(A55_STL_BM68_JMP21_LSL_4),
          [imm0] "I"(A55_STL_BM68_IMM_VALUE_0),
          [init1] "r"(A55_STL_BM68_INIT_1_VALUE),
          [rot] "I"(A55_STL_ROTATE_1)
        : "x9", "x10", "x11", "x12", "x29", "memory"
    );
    if (signature != golden) {
        pr_err("BM68 signature mismatch: got=0x%llx, expect=0x%llx\n", signature, golden);
        fail = -1;
    }
    return fail;
}

// BM15: ADR test
static int test_bm15(void) {
    uint64_t diff1, diff2, diff3, diff4, diff5, diff6, diff7, diff8;
    uint64_t golden1 = A55_STL_BM15_GOLDEN_VALUE_1;
    uint64_t golden2 = A55_STL_BM15_GOLDEN_VALUE_2;
    int fail = 0;

    asm volatile(
        "100:\n"
        "bl 101f\n"
        "101:\n"
        "adr x10, 100b\n"
        "adr x11, 100b\n"
        "102:\n"
        "adr x12, 101b\n"
        "adr x13, 101b\n"
		"103:\n"
        "adr x14, 102b\n"
        "adr x15, 102b\n"
		"104:\n"
        "adr x16, 103b\n"
        "adr x17, 103b\n"
        "mov x18, lr\n"
        "sub %[d1], x18, x10\n"
        "sub %[d2], x18, x11\n"
        "sub %[d3], x12, x10\n"
        "sub %[d4], x13, x10\n"
        "sub %[d5], x14, x12\n"
        "sub %[d6], x15, x12\n"
        "sub %[d7], x16, x14\n"
        "sub %[d8], x17, x14\n"
        : [d1] "=r"(diff1), [d2] "=r"(diff2), [d3] "=r"(diff3), [d4] "=r"(diff4),
          [d5] "=r"(diff5), [d6] "=r"(diff6), [d7] "=r"(diff7), [d8] "=r"(diff8)
        :
        : "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "memory"
    );
    if (diff1 != golden1 || diff2 != golden1 || diff3 != golden1 || diff4 != golden1) {
        pr_err("BM15 ADR test failed: diff1~diff4 mismatch\n");
        fail = -1;
    }
    if (diff5 != golden2 || diff6 != golden2 || diff7 != golden2 || diff8 != golden2) {
        pr_err("BM15 ADR test failed: diff5~diff8 mismatch\n");
        fail = -1;
    }
    return fail;
}

#define TEST_BRANCH_BCOND() \
    do { \
        if (test_bm67() != 0) return -1; \
    } while (0)

#define TEST_BRANCH_MISC() \
    do { \
        if (test_bm68() != 0) return -1; \
    } while (0)

#define TEST_BRANCH_ADR() \
    do { \
        if (test_bm15() != 0) return -1; \
    } while (0)

int a55_stl_core_diagnose_p005_n001_c_impl(void) {
    TEST_BRANCH_BCOND();
    TEST_BRANCH_MISC();
    TEST_BRANCH_ADR();
    return 0;
}

void a55_stl_core_diagnose_p005(a55_stl_state_t *fctlr_base) {
    if (a55_stl_core_diagnose_p005_n001_c_impl() != 0) {
        pr_info("A55 STL Core P005: Test failed\n");
    } else {
        pr_info("A55 STL Core P005: Test passed\n");
    }
}