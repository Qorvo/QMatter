/*
 * Copyright (c) 2020, Qorvo Inc
 *
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by
 * Qorvo Inc.
 *
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 * CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * QORVO INC. SHALL NOT, IN ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 * FOR ANY REASON WHATSOEVER.
 *
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 */

/**
 * @file gpSched_il.c
 * @brief GP Scheduler integration layer implementation
 *
 * This file implements the for gpSched integration layer interface APIs
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_SCHED

#include "hal.h"
#include "gpLog.h"
#include "gpUtils.h"
#include "gpSched.h"
#include "gpSched_defs.h"

/*****************************************************************************
 *                    External Function Prototypes
 *****************************************************************************/

extern void Application_Init( void );

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifndef GP_SCHED_EXTERNAL_MAIN
void gpSched_Main_Init(void)
{
    HAL_INITIALIZE_GLOBAL_INT();

    // Hardware initialization
    HAL_INIT();

    // Radio interrupts that occur will only be handled later on in the main loop
    // Other interrupt source do not trigger any calls to blocks that are not initialized yet
    HAL_ENABLE_GLOBAL_INT();

    Application_Init();

    GP_UTILS_DUMP_STACK_POINTER();
    GP_UTILS_CHECK_STACK_PATTERN();
    GP_UTILS_CHECK_STACK_POINTER();
}
#endif

#ifndef GP_SCHED_EXTERNAL_MAIN
MAIN_FUNCTION_RETURN_TYPE MAIN_FUNCTION_NAME(void)
{
    //the line below destroys the stack. so don't call it from a function!
    GP_UTILS_INIT_STACK();
    gpSched_Main_Init();
    GP_UTILS_CPUMON_INIT();
    for (;;)
    {
        GP_UTILS_CPUMON_NEW_SCHEDULER_LOOP();
        gpSched_Main_Body();

#if GP_SCHED_NR_OF_IDLE_CALLBACKS > 0
        /* When new work can be indicated to an external stack/scheduler at many locations in our stack it is
         * easier and less error-prone to service that stack here than at all these different code locations.
         * Continuously rescheduling a handler event is not a good alternative because this prohibits sleeping. */
        /* Call all registered post processing callbacks*/
        gpSched_PostProcessIdle();
        GP_UTILS_CPUMON_PROCDONE(POSTPROCESSING);
#endif /* GP_SCHED_NR_OF_IDLE_CALLBACKS > 0*/
    }

    return MAIN_FUNCTION_RETURN_VALUE;
}
#endif

void gpSched_NotifySchedTask(void)
{
    /* dummy function */
    return;
}

void Sched_Integration_Init(void)
{
    /* dummy */
    return;
}
void Sched_Integration_DeInit(void)
{
    /* dummy */
    return;
}
void gpSched_Trigger(void)
{
    /* dummy */
    return;
}
