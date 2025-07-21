/*
 *  The confidential and proprietary information contained in this file may
 *  only be used by a person authorised under and to the extent permitted
 *  by a subsisting licensing agreement from ARM Limited or its affiliates.
 *
 *  (C) COPYRIGHT 2021-2022 ARM Limited or its affiliates. ALL RIGHTS RESERVED
 *
 *  This entire notice must be reproduced on all copies of this file
 *  and copies of this file may only be made by a person if such person is
 *  permitted to do so under the terms of a subsisting license agreement
 *  from ARM Limited or its affiliates.
 */

#ifndef __A55_STL_GLOBAL_DEFS_H
#define __A55_STL_GLOBAL_DEFS_H

// Definition of CPUs number in the cluster
#define A55_STL_CPUS_NUM                      (4u)

// CTR_EL0 register value
#define A55_STL_CTR_EL0                       (0x0000000084448004)

// DCZID_EL0 register value
#define A55_STL_DCZID_EL0                     (0x0000000000000004)

// ID_AA64AFR0_EL1 register value
#define A55_STL_ID_AA64AFR0_EL1               (0x0000000000000000)

// ID_AA64DFR0_EL1 register value
#define A55_STL_ID_AA64DFR0_EL1               (0x0000000010305408)

// ID_AA64DFR1_EL1 register value
#define A55_STL_ID_AA64DFR1_EL1               (0x0000000000000000)

// ID_AA64ISAR0_EL1 register value
#define A55_STL_ID_AA64ISAR0_EL1              (0x0000100010210000)

// ID_AA64ISAR1_EL1 register value
#define A55_STL_ID_AA64ISAR1_EL1              (0x0000000000100001)

// ID_AA64MMFR0_EL1 register value
#define A55_STL_ID_AA64MMFR0_EL1              (0x0000000000101122)

// ID_AA64MMFR1_EL1 register value
#define A55_STL_ID_AA64MMFR1_EL1              (0x0000000010212122)

// ID_AA64PFR0_EL1 register value
#define A55_STL_ID_AA64PFR0_EL1               (0x0000000011112222)

// ID_AA64PFR1_EL1 register value
#define A55_STL_ID_AA64PFR1_EL1               (0x0000000000000010)

// ID_DFR0_EL1 register value
#define A55_STL_ID_DFR0_EL1                   (0x0000000004010088)

// ID_ISAR0_EL1 register value
#define A55_STL_ID_ISAR0_EL1                  (0x0000000002101110)

// ID_ISAR1_EL1 register value
#define A55_STL_ID_ISAR1_EL1                  (0x0000000013112111)

// ID_ISAR2_EL1 register value
#define A55_STL_ID_ISAR2_EL1                  (0x0000000021232042)

// ID_ISAR3_EL1 register value
#define A55_STL_ID_ISAR3_EL1                  (0x0000000001112131)

// ID_ISAR4_EL1 register value
#define A55_STL_ID_ISAR4_EL1                  (0x0000000000011142)

// ID_ISAR5_EL1 register value
#define A55_STL_ID_ISAR5_EL1                  (0x0000000001010001)

// ID_MMFR0_EL1 register value
#define A55_STL_ID_MMFR0_EL1                  (0x0000000010201105)

// ID_MMFR1_EL1 register value
#define A55_STL_ID_MMFR1_EL1                  (0x0000000040000000)

// ID_MMFR2_EL1 register value
#define A55_STL_ID_MMFR2_EL1                  (0x0000000001260000)

// ID_MMFR3_EL1 register value
#define A55_STL_ID_MMFR3_EL1                  (0x0000000002122211)

// ID_PFR0_EL1 register value
#define A55_STL_ID_PFR0_EL1                   (0x0000000010000131)

// ID_PFR1_EL1 register value
#define A55_STL_ID_PFR1_EL1                   (0x0000000010011011)

// CLIDR_EL1 register value
#define A55_STL_CLIDR_EL1                     (0x00000000C3000123)

// MIDR_EL1 register value
#define A55_STL_MIDR_EL1                      (0x00000000412FD050)

// MVFR0_EL1 register value
#define A55_STL_MVFR0_EL1                     (0x0000000010110222)

// MVFR1_EL1 register value
#define A55_STL_MVFR1_EL1                     (0x0000000013211111)

// MVFR2_EL1 register value
#define A55_STL_MVFR2_EL1                     (0x0000000000000043)

// REVIDR_EL1 register value
#define A55_STL_REVIDR_EL1                    (0x0000000000000000)

// ID_AA64AFR1_EL1 register value
#define A55_STL_ID_AA64AFR1_EL1               (0x0000000000000000)

// ICH_VTR_EL2 register value
#define A55_STL_ICH_VTR_EL2                   (0x0000000090280003)

// ID_PFR2_EL1 register value
#define A55_STL_ID_PFR2_EL1                   (0x0000000000000010)

// CPUCFR_EL1 register value
#define A55_STL_CPUCFR_EL1                    (0x0000000000000001)

// ERRIDR_EL1 register value
#define A55_STL_ERRIDR_EL1                    (0x0000000000000002)

// ID_AA64MMFR2_EL1 register value
#define A55_STL_ID_AA64MMFR2_EL1              (0x0000000000001011)

// ID_ISAR6_EL1 register value
#define A55_STL_ID_ISAR6_EL1                  (0x0000000000000010)

// ID_MMFR4_EL1 register value
#define A55_STL_ID_MMFR4_EL1                  (0x0000000000021110)

// LORID_EL1 register value
#define A55_STL_LORID_EL1                     (0x0000000000040004)

// ERR0FR register value
#define A55_STL_ERR0FR                        (0x000000000000A9A6)

//L1 data cache size: 16, 32 or 64 KB
#define A55_STL_L1_DATA_CACHE_SIZE            (32u)

//L1 instruction cache size: 16, 32 or 64 KB
#define A55_STL_L1_INSTR_CACHE_SIZE           (32u)


// A55 STL Force Fail Functionality: comment out the following code statement
// to enable this functionality, comment out to disable it.
// #define A55_STL_FAIL_TEST

#define A55_STL_PA_1        (0x0000000080000000)
#define A55_STL_PA_2        (0x00000000FFFFF000)
#define A55_STL_PA_3        (0x0000000080500000)
#define A55_STL_PA_4        (0x0000000080100000)
#define A55_STL_PA_5        (0x00000000BFAAA000)
#define A55_STL_PA_6        (0x0000000080200000)
#define A55_STL_PA_7        (0x00000000C0000000)
#define A55_STL_PA_8        (0x0000000080400000)
#define A55_STL_PA_9        (0x00000000BFFFF000)
#define A55_STL_PA_10       (0x0000000080300000)
#define A55_STL_PA_11       (0x00000000C0000000)


#endif
