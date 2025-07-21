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

#ifndef __A55_STL_H
#define __A55_STL_H

// Definitions of the exception levels
#define  A55_STL_EL0_EL1  (1u)
#define  A55_STL_EL2      (2u)
#define  A55_STL_EL3      (3u)
// Defensive error value
#define A55_STL_UNKNOWN_ERR   (0xFFFFu)

// Definitions of execution mode
// Out of Reset execution mode
#define A55_STL_OOR_MODE       (1u)
// OnLine execution mode
#define A55_STL_ONL_MODE       (2u)

// Result constants
#define A55_STL_PASS     (0x900Du)
#define A55_STL_FAIL     (0xBADu)
#define A55_STL_INV      -1

// A55 STL exception level constants
typedef enum {
    a55_stl_el0_el1 = A55_STL_EL0_EL1,
    a55_stl_el2 = A55_STL_EL2,
    a55_stl_el3 = A55_STL_EL3,
    a55_stl_el_unknown_err = A55_STL_UNKNOWN_ERR,
} a55_stl_el_t;

// A55 STL operating mode constants
typedef enum {
    a55_stl_oor_mode = A55_STL_OOR_MODE,
    a55_stl_onl_mode = A55_STL_ONL_MODE,
} a55_stl_mode_t;

// A55 STL test result codes
typedef enum {
    a55_stl_inval = A55_STL_INV,
    a55_stl_pass = A55_STL_PASS,
    a55_stl_fail = A55_STL_FAIL,
} a55_stl_result_t;

// A55 STL memory mapped registers structure type definition
typedef struct a55_stl_state {
    // Offset: 0x000 (R/W) Fault Control Register
    volatile uint32_t fctlr;
    // Offset: 0x004 (R/W) Fault Partition Identifier Register
    volatile uint32_t fpir;
    // Offset: 0x008 (R/W) Fault Failure Mode Identification Register
    volatile uint32_t ffmir;
} a55_stl_state_t;

// A55 STL structure type definition
typedef struct a55_stl {
    // Pointer to the initial address of the heap memory area
    void * heap;
    // ID of the first SW Part to be run, in the array containing the scheduled SW Parts
    uint32_t start;
    // ID of the last SW Part to be run, in the array containing the scheduled SW Parts
    uint32_t end;
    // Execution mode (Out of Reset or OnLine)
    a55_stl_mode_t mode;
    // Exception level
    a55_stl_el_t el;
    // Pointer to the base address of memory area used to save memory mapped registers
    a55_stl_state_t *stateRegsBase;
}a55_stl_t;

//====================================================================================================================================
//   Function: A55_STL
//      A55 STL Main API
//
//   Parameters:
//      None
//
//   Returns: a55_stl_result_t
//     Global pass/fail result of all executed SW parts:
//        a55_stl_pass (0x900D) No dangerous faults detected;
//        a55_stl_fail (0xBAD) Potentially dangerous scenario detected
//====================================================================================================================================

a55_stl_result_t A55_STL(void);

//====================================================================================================================================
//   Function: A55_STL_init
//      Function used to initialize memory areas for memory mapped register and heap
//
//   Parameters:
//      base   - Base address of the memory area used for tracking STL-state information and reporting failure-modes (memory mapped registers)
//      heap   - Address of the heap allocated for the STL. This area is used for context switch
//
//   Returns: a55_stl_result_t
//      Global pass/fail result of library initialization:
//         a55_stl_pass (0x900D) System successfully initialized;
//         a55_stl_fail (0xBAD) System initialization failed
//====================================================================================================================================

a55_stl_result_t A55_STL_init(a55_stl_state_t * base, void * heap);

//====================================================================================================================================
//   Function: A55_STL_setParam
//      Initialize the behavior of the A55 STL
//
//   Parameters:
//      mode  - Indicates the mode of execution: Out Of Reset (OOR) if it is set to 1, OnLine (OnL) if it is set to 2
//      start - Indicates the starting SW Part ID
//      last  - Indicates the last SW Part ID
//
//   Returns: a55_stl_result_t
//      Global pass/fail result of parameters setting:
//         a55_stl_pass (0x900D) STL successfully configured;
//         a55_stl_fail (0xBAD) STL configuration failed
//         a55_stl_inval (-1) Invalid parameter passed by user
//====================================================================================================================================

a55_stl_result_t A55_STL_setParam(a55_stl_mode_t mode, uint32_t start, uint32_t last);

//====================================================================================================================================
//   Function: A55_stl_el0
//      A55 STL Main API for EL0
//
//   Parameters:
//      mode  - Indicates the mode of execution: Out Of Reset (OOR) if it is set to 1, OnLine (OnL) if it is set to 2
//      start - Indicates the starting test ID
//      last  - Indicates the last test ID
//      base  - Base address of the memory area used for tracking STL-state information and reporting failure-modes
//      cpuId - Indicates the ID of the current CPU
//      heap  - Address of the heap allocated for the STL. This area is used for context switch
//
//   Returns: a55_stl_result_t
//      Global pass/fail result of all executed SW parts:
//         a55_stl_pass (0x900D) No dangerous faults detected;
//         a55_stl_fail (0xBAD) Potentially dangerous scenario detected
//         a55_stl_inval (-1) Invalid parameter passed by user
//====================================================================================================================================

a55_stl_result_t A55_STL_el0(a55_stl_mode_t mode, uint32_t start, uint32_t last, a55_stl_state_t * base, uint32_t cpuId, void * heap);

#endif

