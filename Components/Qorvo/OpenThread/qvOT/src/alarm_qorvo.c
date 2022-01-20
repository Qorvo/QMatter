/*
 * Copyright (c) 2017-2018, 2020-2021, Qorvo Inc
 *
 * alarm_qorvo.c
 *   This file contains the implementation of the qorvo alarm api for openthread.
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
 * $Header: //depot/release/Embedded/Applications/P959_OpenThread/v1.1.23.1/comps/qvOT/src/alarm_qorvo.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

#define GP_COMPONENT_ID GP_COMPONENT_ID_QVOT

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpAssert.h"
#include "gpLog.h"
#include "gpSched.h"

#include "alarm_qorvo.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define QORVO_ALARM_MILLI_WRAP        ((UInt32)(0xFFFFFFFF / 1000))
#define QORVO_ALARM_KEEP_ALIVE_PERIOD ((UInt32)QORVO_ALARM_MILLI_WRAP - 10000)

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static uint16_t qorvoAlarmWrapCounter;
static uint32_t qorvoAlarmPrev;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void qorvoAlarmResetWrapCounter(void);
static void qorvoAlarmKeepAlive(void);
static void qorvoAlarmUpdateWrapAround(UInt32 now);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void qorvoAlarmResetWrapCounter(void)
{
    qorvoAlarmWrapCounter = 0;
    qorvoAlarmPrev = gpSched_GetCurrentTime();
    GP_LOG_PRINTF("QorvoAlarm: internal timer reset. cnt=%u", 0, qorvoAlarmWrapCounter);
    gpSched_UnscheduleEventArg((gpSched_EventCallback_t)qorvoAlarmKeepAlive, NULL);
    gpSched_ScheduleEventInSecAndUs(QORVO_ALARM_KEEP_ALIVE_PERIOD / 1000, (QORVO_ALARM_KEEP_ALIVE_PERIOD % 1000) * 1000, (gpSched_EventCallback_t)qorvoAlarmKeepAlive, NULL);
}

void qorvoAlarmKeepAlive(void)
{
    GP_LOG_PRINTF("QorvoAlarm: Keep Alive triggered", 0);
    qorvoAlarmGetTimeMs();
}

void qorvoAlarmUpdateWrapAround(UInt32 now)
{
    if(now < qorvoAlarmPrev)
    {
        qorvoAlarmWrapCounter += (qorvoAlarmWrapCounter < 1000 ? 1 : -1000);
        GP_LOG_PRINTF("QorvoAlarm: internal timer wrap. cnt=%u", 0, qorvoAlarmWrapCounter);
        GP_ASSERT_DEV_INT(qorvoAlarmWrapCounter <= 1000);
    }
    qorvoAlarmPrev = now;

    gpSched_UnscheduleEventArg((gpSched_EventCallback_t)qorvoAlarmKeepAlive, NULL);
    gpSched_ScheduleEventInSecAndUs(QORVO_ALARM_KEEP_ALIVE_PERIOD / 1000, (QORVO_ALARM_KEEP_ALIVE_PERIOD % 1000) * 1000, (gpSched_EventCallback_t)qorvoAlarmKeepAlive, NULL);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void qorvoAlarmInit(void)
{
    qorvoAlarmResetWrapCounter();
}

uint32_t qorvoAlarmGetTimeMs(void)
{
    UInt32 now;

    now = gpSched_GetCurrentTime();

    qorvoAlarmUpdateWrapAround(now);
    return (uint32_t)(now / 1000 + qorvoAlarmWrapCounter * QORVO_ALARM_MILLI_WRAP);
}

bool qorvoAlarmUnScheduleEventArg(qorvoAlarmCallback_t callback, void* arg)
{
    return (bool)gpSched_UnscheduleEventArg((gpSched_EventCallback_t)callback, arg);
}

void qorvoAlarmScheduleEventArg(uint32_t rel_time, qorvoAlarmCallback_t callback, void* arg)
{
    gpSched_ScheduleEventInSecAndUs(rel_time / 1000, (rel_time % 1000) * 1000, (gpSched_EventCallback_t)callback, arg);
}
