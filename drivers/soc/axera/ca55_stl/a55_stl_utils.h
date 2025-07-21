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

#ifndef __A55_STL_UTILS_H
#define __A55_STL_UTILS_H

// Zero value constant
#define A55_STL_ZERO_VALUE            (0x00000000)
// One value constant
#define A55_STL_ONE_VALUE             (0x00000001u)
// Three value constant
#define A55_STL_THREE_VALUE           (0x00000003)
// Empty value constant
#define A55_STL_EMPTY_VALUE           (0xDEADBEEFu)
#define A55_STL_EMPTY_BYTE            (0x000000FFu)

// Memory mapped register execution mode
#define A55_STL_FCTLR_STATUS_MODE     (0x00000000u)
#define A55_STL_FPIR_MTID_MODE        (0x00000001u)
#define A55_STL_FPIR_STID_MODE        (0x00000002u)

// FCTLR bit access masks
#define A55_STL_FCTLR_STATUS_MSK      (0x0000000Fu)
#define A55_STL_FCTLR_STATUS_PING     (0x00000002)
#define A55_STL_FCTLR_STATUS_DONE     (0x00000003u)
#define A55_STL_FCTLR_STATUS_IDLE     (0x00000000u)
#define A55_STL_FCTLR_STATUS_FAIL     (0x00000004)
#define A55_STL_FCTLR_STATUS_PDONE    (0x00000005)
#define A55_STL_FCTLR_STATUS_MSK_R    (0xFFFFFFF0)
#define A55_STL_FCTLR_STATUS_SHF      (0x00000000u)


// FPIR bit access masks
#define A55_STL_FPIR_MTID_MSK         (0x3F000000u)
#define A55_STL_FPIR_STID_MSK         (0x00FF0000u)
#define A55_STL_FPIR_MTID_CORE        (0x00000001u)
#define A55_STL_FPIR_MTID_GIC         (0x00000002u)
#define A55_STL_FPIR_MTID_MMU         (0x00000003u)
#define A55_STL_FPIR_MTID_SHF         (0x00000018u)

#define A55_STL_FPIR_STID_SHF         (0x00000010u)
// FFMIR bit access masks
#define A55_STL_FFMIR_FMID_DATA_COR   (0x00000004)
#define A55_STL_FFMIR_FMID_EXCEPTION  (0x00000002)
#define A55_STL_FFMIR_OFFSET          (0x00000008)
#define A55_STL_FFMIR_FMID_CLR_MSK    (0xFFFFFFF0)

#ifdef CCODE

// Definition of the null pointer
#define A55_STL_NULL                  (0)

//===========================================================================================
//   Function: a55_stl_getcpuid
//      Function to get CPU ID value from MPIDR_EL1
//
//   Parameters:
//      None
//
//   Returns:
//      Current CPU ID
//===========================================================================================

uint32_t a55_stl_getcpuid(void);

//===========================================================================================
//   Function: a55_stl_get_el
//      Function to get the current exception level from CurrentEL register
//
//   Parameters:
//      None
//
//   Returns:
//      Current exception level value
//===========================================================================================

a55_stl_el_t a55_stl_get_el(void);

#endif
#endif
