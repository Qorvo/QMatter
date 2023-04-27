/*
 * Copyright (c) 2017-2021, Qorvo Inc
 *
 * platform_qorvo.c
 *   This file contains the implementation of the qorvo platform api for openthread.
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
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

#ifdef GP_DIVERSITY_FREERTOS
#define APP_TASK_NAME  "OTHR"
#define APP_STACK_SIZE (1500)

extern int main(void);
#endif // GP_DIVERSITY_FREERTOS

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "platform_qorvo.h"
#include "gpLog.h"
#include "hal.h"
#include "gpSched.h"
#include "gpUtils.h"
#include "gpBaseComps.h"

#ifdef GP_DIVERSITY_FREERTOS
#include "gpCom.h"
#include "FreeRTOS.h"
#endif // GP_DIVERSITY_FREERTOS

#if defined(GP_DIVERSITY_JUMPTABLES)
#include "gpJumpTables.h"
#endif // defined(GP_DIVERSITY_JUMPTABLES)

#ifdef GP_DIVERSITY_FREERTOS
TaskHandle_t sAppTaskHandle;
StackType_t appStack[APP_STACK_SIZE];
StaticTask_t appTaskStruct;
#endif // GP_DIVERSITY_FREERTOS

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*
    Inits basic components
*/
static void qorvoPlatInitBase()
{
    HAL_INITIALIZE_GLOBAL_INT();

    // Hardware initialisation
    HAL_INIT();

    // Radio interrupts that occur will only be handled later on in the main loop
    // Other interrupt source do not trigger any calls to blocks that are not initialized yet
    HAL_ENABLE_GLOBAL_INT();
#ifdef GP_DIVERSITY_FREERTOS
    gpSched_Init();
#endif // GP_DIVERSITY_FREERTOS

    gpBaseComps_StackInit();
    GP_UTILS_DUMP_STACK_POINTER();
    GP_UTILS_CHECK_STACK_PATTERN();
    GP_UTILS_CHECK_STACK_POINTER();


#if defined(GP_DIVERSITY_JUMPTABLES)
    gpJumpTables_GetRomVersionFromRom();
#endif // defined(GP_DIVERSITY_JUMPTABLES)
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void qorvoPlatMainLoop(bool canGoToSleep)
{
#ifndef GP_DIVERSITY_FREERTOS
    if(canGoToSleep)
    {
        gpSched_GoToSleep();
    }
    gpSched_Main_Body();
#endif // GP_DIVERSITY_FREERTOS
}

void qorvoPlatInit(qorvoPlatGotoSleepCheckCallback_t gotoSleepCheckCallback)
{
#ifndef GP_DIVERSITY_FREERTOS
    qorvoPlatInitBase();
#else  // GP_DIVERSITY_FREERTOS
    // System init already done. qorvoPlatInit runs in FreeRTOS task context.
#endif // GP_DIVERSITY_FREERTOS

    gpSched_SetGotoSleepCheckCallback((gpSched_GotoSleepCheckCallback_t)gotoSleepCheckCallback);
}


#ifdef GP_DIVERSITY_FREERTOS
/*
    Openthread FreeRTOS task function
*/
static void ThreadTaskMain(void* pvParameter)
{
    main();
}

/*
    Platform initialization function for FreeRTOS.
    Starts ThreadTaskMain as FreeRTOS task
    This code assumes main() is defined outside library(i.e. OT cli app main).
    This code assumes no openthread functions were called before
        and main() will init OT stack using platform implementation(i.e. qorvoPlatInit).
*/
int qorvoPlatInitFreeRTOS(void)
{
    qorvoPlatInitBase();

    sAppTaskHandle = xTaskCreateStatic(ThreadTaskMain,
                                       APP_TASK_NAME,
                                       APP_STACK_SIZE,
                                       NULL,
                                       tskIDLE_PRIORITY,
                                       appStack,
                                       &appTaskStruct);
    if(sAppTaskHandle == NULL)
    {
        GP_LOG_SYSTEM_PRINTF("Failed to allocate app sAppTaskHandle", 0);
        return 1;
    }

    /* Start FreeRTOS */
    vTaskStartScheduler();

    return 0;
}
#endif // GP_DIVERSITY_FREERTOS
