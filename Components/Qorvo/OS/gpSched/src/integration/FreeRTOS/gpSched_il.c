/*
 * Copyright (c) 2020, Qorvo Inc
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
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "gpUtils.h"
#include "gpSched.h"
#include "gpHal_ES.h"
#include "gpSched_defs.h"

#include "gpLog.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_SCHED_TASK_NAME       ("taskGPSched")
#define GP_SCHED_TASK_PRIORITY   (configMAX_PRIORITIES - 1)
#define GP_SCHED_TASK_STACK_SIZE ((4 * 1024) / 4)

#define GP_SCHED_TASK_NOTIFY_EVENTQ_MASK    (0x1UL)
#define GP_SCHED_TASK_NOTIFY_TERMINATE_MASK (0x2UL)
#define GP_SCHED_TASK_NOTIFY_ALL_MASK       (GP_SCHED_TASK_NOTIFY_EVENTQ_MASK | GP_SCHED_TASK_NOTIFY_TERMINATE_MASK)

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/** @brief gpSched FreeRTOS task handle */
static TaskHandle_t gpSched_TaskHandle;
#if configSUPPORT_STATIC_ALLOCATION
/** @brief gpSched FreeRTOS task info */
static StaticTask_t gpSched_TaskInfo;
/** @brief gpSched FreeRTOS Stack allocation */
static StackType_t gpSched_TaskStack[GP_SCHED_TASK_STACK_SIZE];
#endif //GP_FREERTOS_DIVERSITY_HEAP

/** @brief Idle tick tracking */
static UInt32 gpSched_IdleCnt;
/** @brief Bool to signal initialisation done before the main loop has passed. */
static Bool gpSched_AppInitDone;
/** @brief ID of claimed HW Absolute Event for kick of gpSched task */
static gpHal_AbsoluteEventId_t gpSched_ESTimerId;


/*****************************************************************************
 *                    External Function Prototypes
 *****************************************************************************/

extern void Application_Init(void);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

#define Sched_WriteDbgInfo(line, arg1, arg2, arg3)

static void Sched_ScheduleESTimer(UInt32 nextEventTime)
{
    UInt32 currTime = 0;
    UInt32 absTime;
    UInt8 control = 0;

    if(nextEventTime)
    {
        DISABLE_GP_GLOBAL_INT();

        GP_ES_SET_EVENT_RESULT(control, gpHal_EventResultInvalid);
        GP_ES_SET_EVENT_STATE(control, gpHal_EventStateScheduled);
        HAL_TIMER_GET_CURRENT_TIME_1US(currTime);
        absTime = nextEventTime + currTime;
        gpHal_RefreshAbsoluteEvent(gpSched_ESTimerId, absTime, control);

        ENABLE_GP_GLOBAL_INT();
        Sched_WriteDbgInfo(__LINE__, currTime, nextEventTime, absTime);
    }
}

static void Sched_SetupESTimer(void)
{
    gpHal_AbsoluteEventDescriptor_t ev;

    gpSched_ESTimerId = gpHal_GetAbsoluteEvent();
    GP_ASSERT_DEV_EXT(GPHAL_ES_ABSOLUTE_EVENT_ID_INVALID != gpSched_ESTimerId);

    MEMSET(&ev, 0, sizeof(ev));
    ev.type = GP_WB_ENUM_EVENT_TYPE_DUMMY;
    /* we need ex_itl when waking up very fast */
    ev.executionOptions = GP_ES_EXECUTION_OPTIONS_EXECUTE_IF_TOO_LATE;
    ev.interruptOptions = GP_ES_INTERRUPT_OPTIONS_MASK;
    gpHal_GetTime(&ev.exTime);
    GP_ES_SET_EVENT_STATE(ev.control, gpHal_EventStateScheduled);

    gpHal_ScheduleAbsoluteEvent(&ev, gpSched_ESTimerId);
    gpHal_UnscheduleAbsoluteEvent(gpSched_ESTimerId);
    gpHal_EnableAbsoluteEventCallbackInterrupt(gpSched_ESTimerId, true);
}

static void Sched_Main(void* params)
{
#ifndef GP_SCHED_EXTERNAL_MAIN
    HAL_INITIALIZE_GLOBAL_INT();

    // Hardware initialization
    HAL_INIT();

    HAL_ENABLE_GLOBAL_INT();

    Application_Init();
#endif //GP_SCHED_EXTERNAL_MAIN

    Sched_SetupESTimer();

    GP_UTILS_CPUMON_INIT();

    gpSched_AppInitDone = true;
    /* scheduler task loop */
    for(;;)
    {
        UInt32 notificationVal;
        UInt32 nextEventTime = 0UL;
        Bool timerScheduled = false;
        /* wait for the notification */
        xTaskNotifyWait(0x0UL, GP_SCHED_TASK_NOTIFY_ALL_MASK, &notificationVal, portMAX_DELAY);
        if((notificationVal & GP_SCHED_TASK_NOTIFY_EVENTQ_MASK) != 0UL)
        {
            GP_UTILS_CPUMON_NEW_SCHEDULER_LOOP();
            gpSched_Main_Body();
            while((!gpSched_EventQueueEmpty() && (timerScheduled == false)) || (HAL_RADIO_INT_CHECK_IF_OCCURED()))
            {
                gpSched_Main_Body();
                if(!gpSched_EventQueueEmpty())
                {
                    nextEventTime = gpSched_GetTimeToNextEvent();
                    if((nextEventTime > 0) && (timerScheduled == false))
                    {
                        Sched_WriteDbgInfo(__LINE__, 0, nextEventTime, 0);
                        Sched_ScheduleESTimer(nextEventTime);
                        timerScheduled = true;
                    }
                }
                else
                {
                    Sched_WriteDbgInfo(__LINE__, 0, 0, 0);
                }
            }
#if GP_SCHED_NR_OF_IDLE_CALLBACKS > 0
            gpSched_PostProcessIdle();
#endif
        }
        /* terminate signal received */
        if((notificationVal & GP_SCHED_TASK_NOTIFY_TERMINATE_MASK) != 0UL)
        {
            break;
        }
    }
    /* shall never come here ideally. For safety reason, delete the task before returning */
    vTaskDelete(NULL);
}

/*****************************************************************************
 *                    FreeRTOS weak overrides
 *****************************************************************************/

/* Supress the tick interrupt and enter into sleep */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    gpSched_IdleCnt++;
    if(gpSched_AppInitDone)
    {
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        gpSched_GoToSleep();
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    }
}

void vApplicationIdleHook(void)
{
    //Kick watchdog in FreeRTOS idle loop
    HAL_WDT_RESET();
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

/* This function can be called either from interrupt context (handler mode)
 * or thread context (thread mode) to signal to gpSched task. This functinal
 * internally takes care of either sending signal to task using xTaskNotifyFromISR()
 * or xTaskNotify() function of FreeRTOS
 */
void gpSched_NotifySchedTask(void)
{
    xPSR_Type psr;
    psr.w = __get_xPSR();
    if(psr.b.ISR != 0)
    {
        xTaskNotifyFromISR(gpSched_TaskHandle, GP_SCHED_TASK_NOTIFY_EVENTQ_MASK, eSetBits, NULL);
    }
    else
    {
        xTaskNotify(gpSched_TaskHandle, GP_SCHED_TASK_NOTIFY_EVENTQ_MASK, eSetBits);
    }
}

Bool gpSched_InitTask(void)
{
    gpSched_AppInitDone = false;

#if configSUPPORT_STATIC_ALLOCATION
    gpSched_TaskHandle = xTaskCreateStatic(Sched_Main,
                                           GP_SCHED_TASK_NAME,
                                           GP_SCHED_TASK_STACK_SIZE,
                                           NULL,
                                           GP_SCHED_TASK_PRIORITY,
                                           gpSched_TaskStack,
                                           &gpSched_TaskInfo);
#else
    (void)xTaskCreate(Sched_Main,
                      GP_SCHED_TASK_NAME,
                      GP_SCHED_TASK_STACK_SIZE,
                      NULL,
                      GP_SCHED_TASK_PRIORITY,
                      &gpSched_TaskHandle);
#endif // configSUPPORT_STATIC_ALLOCATION
    return (NULL != gpSched_TaskHandle);
}

#ifndef GP_SCHED_EXTERNAL_MAIN
MAIN_FUNCTION_RETURN_TYPE MAIN_FUNCTION_NAME(void)
{
    Bool initSuccess;

    initSuccess = gpSched_InitTask();

    /* Start the tasks and timer running. */
    if(initSuccess)
    {
        vTaskStartScheduler();
    }
    return MAIN_FUNCTION_RETURN_VALUE;
}
#endif //GP_SCHED_EXTERNAL_MAIN

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
