 /*
 * Copyright (c) 2017-2018, Qorvo Inc
 *
 * gpHal_Calibration.c
 * Support periodic recalibration of hardware.
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
 * $Header: //depot/release/Embedded/Components/Qorvo/HAL_RF/v2.10.2.1/comps/gphal/src/gpHal_Calibration.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL
//#define GP_LOCAL_LOG

#include "gpHal.h"
#include "gpHal_Calibration.h"
#include "gpHal_kx_Ipc.h"
#include "gpAssert.h"
#include "gpLog.h"
#include "gpSched.h"
#include "gpHal_Phy.h"
#include "gpStat.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

// Interval in which chip will wake up to do a calibration
// In non scheduled case, calibrations will happen when chip wakes
// up each time and when it handles interrupts
// The max temperature slope supported is 1 C change per 8 s. So setting
// the temperature based calibration tasks to run every min in the worst
// case (if not run in an unscheduled manner already) should capture any
// 10C change
#define CALIBRATION_TASK_SCHED_INTERVAL_US 60000000

// Number of temperature measurements to average to get a stable measurement value
#define NOF_TEMP_MEASUREMENTS_TO_AVG 4
/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    gpHal_CalibrationTask_t task;
    gpHal_cbCalibrationHandler_t cbHandler;
    UInt32 nextCalibrationTime;
    Q8_8 lastCalibrationTemperature;
    Bool IsFirstCalibrationAfterWakeup;
} gpHal_CalibrationTaskInfo_t;


/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// Number of calibration tasks defined.
static UInt8   HalCalibration_NrOfTasks;

// Last measured temperature.
static Q8_8  lastMeasuredTemperature;

// Time of next check for time based events.
static UInt32  HalCalibration_NextCheckTime;

// Time of next check for temperature based  events.
static UInt32  HalCalibrationTempBased_NextCheckTime;

// Calibration task data.
static gpHal_CalibrationTaskInfo_t HalCalibration_TaskInfo[GP_HAL_CALIBRATION_MAX_TASKS];

// Pending calibration on next wakeup
static UInt8 HalCalibration_PendingOnWakeup;

// First calibration after wakeup
static UInt8 HalCalibration_FirstAfterWakeup;

// Variables used for averaging multiple temperature measurements
static UInt8 temperatureMeasurementCounter = 0;
static Int32 temperatureSum = 0;

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

static Bool gpHal_GetCalibrationPendingOnWakeup(UInt8 calTaskId)
{
    return ((HalCalibration_PendingOnWakeup & (1 << calTaskId)) != 0);
}


static void Hal_TimeBasedCalibration(void)
{
    UIntLoop i;
    UInt32 currentTime;
    gpHal_CalibrationFlags_t flags;
    Bool recal;

    HAL_TIMER_GET_CURRENT_TIME_1US(currentTime);
    // Loop over calibration tasks.
    for (i = 0; i < HalCalibration_NrOfTasks; i++)
    {
        flags = HalCalibration_TaskInfo[i].task.flags;
        recal = false;

        if(gpHal_GetCalibrationPendingOnWakeup(i) && gpHal_CalibrationGetFirstAfterWakeup())
        {
            // Pending calibration on wakeup for this particular task i
            recal = true;
        }
        else if (((flags & GP_HAL_CALIBRATION_FLAG_PERIODIC) != 0) &&
           !GP_SCHED_TIME_COMPARE_LOWER_US(currentTime, HalCalibration_TaskInfo[i].nextCalibrationTime))
        {
            // Max time between calibrations reached.
            recal = true;
        }
        else if ((flags & GP_HAL_CALIBRATION_FLAG_CALIBRATE_ON_CHIP_WAKEUP) &&
                gpHal_CalibrationGetFirstAfterWakeup())
        {
            // This task needs to be calibrated every time chip wakes up from sleep
            recal = true;
        }

        if (recal)
        {
            // Setup for next calibration.
            HalCalibration_TaskInfo[i].nextCalibrationTime = currentTime + HalCalibration_TaskInfo[i].task.calibrationPeriod;
            // Calibrate.
            GP_LOG_PRINTF("Calibrate %u", 0, i);
            HalCalibration_TaskInfo[i].cbHandler(&HalCalibration_TaskInfo[i].task);
        }
    }

    /* If its first time the calibration sequence is run after waking up, clear the flag set
    by reset handler that indicates that */
    if (gpHal_CalibrationGetFirstAfterWakeup())
    {
        gpHal_CalibrationSetFirstAfterWakeup(false);
    }
}

static void Hal_TemperatureBasedCalibration(void)
{
    Q8_8 currentTemperature;
    UIntLoop i;
    gpHal_CalibrationFlags_t flags;
    Bool recal;
    GP_LOG_PRINTF("temp_cal", 0);

#ifdef GP_COMP_HALCORTEXM4
    currentTemperature = halADC_MeasureTemperature();
    if (currentTemperature != GP_HAL_ADC_INVALID_TEMPERATURE)
    {
        if (temperatureMeasurementCounter < NOF_TEMP_MEASUREMENTS_TO_AVG)
        {
            temperatureMeasurementCounter++;
            temperatureSum += currentTemperature;
            return;
        }
        else
        {
            temperatureMeasurementCounter = 0;
            currentTemperature = (Q8_8)(Int32)(temperatureSum / NOF_TEMP_MEASUREMENTS_TO_AVG);
            lastMeasuredTemperature = currentTemperature;
            GP_LOG_PRINTF("CAL: tempcal@%d", 0, Q_PRECISION_DECR8(currentTemperature));
            temperatureSum = 0;
        }
    }
    else
    {
        // temperature reading did not succeed, return
        return;
    }
#else
    currentTemperature = 0;
    lastMeasuredTemperature = 0;
#endif

    // Loop over calibration tasks.
    for (i = 0; i < HalCalibration_NrOfTasks; i++)
    {
        flags = HalCalibration_TaskInfo[i].task.flags;
        recal = false;
        if ((flags & GP_HAL_CALIBRATION_FLAG_TEMPERATURE_SENSITIVE) != 0)
        {
            Q8_8 lastTemperature = HalCalibration_TaskInfo[i].lastCalibrationTemperature;

            UInt16 diffTemperature = ABS(currentTemperature - lastTemperature);
            if (diffTemperature >= HalCalibration_TaskInfo[i].task.temperatureThreshold)
            {
                // Max temperature shift between calibrations reached.
                GP_LOG_PRINTF("diff temp %d", 0, diffTemperature);
                recal = true;
            }
        }

        if (recal)
        {
            // Setup for next calibration.
            HalCalibration_TaskInfo[i].lastCalibrationTemperature = currentTemperature;
            HalCalibration_TaskInfo[i].task.temperature  = currentTemperature;
            // Calibrate.
            GP_LOG_PRINTF("Calibrate %u", 0, i);
            HalCalibration_TaskInfo[i].cbHandler(&HalCalibration_TaskInfo[i].task);
        }
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpHal_SetCalibrationPendingOnWakeup(UInt8 calTaskId)
{
    const UInt8 nofBitsInByte = 8;
    GP_ASSERT_DEV_INT(calTaskId < (sizeof(HalCalibration_PendingOnWakeup) * nofBitsInByte));
    HalCalibration_PendingOnWakeup |= (1 << calTaskId);
}

void gpHal_ClearCalibrationPendingOnWakeup(UInt8 calTaskId)
{
    const UInt8 nofBitsInByte = 8;
    GP_ASSERT_DEV_INT(calTaskId < (sizeof(HalCalibration_PendingOnWakeup) * nofBitsInByte));
    HalCalibration_PendingOnWakeup &= ~(1 << calTaskId);
}

void gpHal_CalibrationSetFirstAfterWakeup(Bool enable)
{
    HalCalibration_FirstAfterWakeup = enable;
}

Bool gpHal_CalibrationGetFirstAfterWakeup()
{
    return HalCalibration_FirstAfterWakeup;
}

void gpHal_InitCalibration(void)
{
    GP_LOG_PRINTF("InitCalibration", 0);
    HalCalibration_NrOfTasks = 0;
    HalCalibration_PendingOnWakeup = 0;
    gpHal_CalibrationSetFirstAfterWakeup(false);
#ifdef GP_COMP_HALCORTEXM4
    hal_InitADC();
#ifndef GP_DIVERSITY_FREERTOS
    hal_EnableSysTick(0x4E200); //10ms at 32MHz and 5ms at 64 MHz
#endif
#endif
    temperatureMeasurementCounter = 0;
    temperatureSum = 0;
}

UInt8 gpHal_CalibrationCreateTask(
        const gpHal_CalibrationTask_t* pTask,
        gpHal_cbCalibrationHandler_t cbHandler)
{
    UInt32 currentTime;
    UInt8 taskId;

    GP_LOG_PRINTF("CalibrationCreateTask", 0);

    GP_ASSERT_DEV_INT(pTask != NULL);
    GP_ASSERT_DEV_INT(cbHandler != NULL);
    GP_ASSERT_DEV_INT(((pTask->flags & GP_HAL_CALIBRATION_FLAG_PERIODIC) == 0) || (pTask->calibrationPeriod <= GP_SCHED_EVENT_TIME_MAX));

    if (HalCalibration_NrOfTasks >= GP_HAL_CALIBRATION_MAX_TASKS)
    {
        return GP_HAL_CALIBRATION_INVALID_TASK_HANDLE;
    }

    HAL_TIMER_GET_CURRENT_TIME_1US(currentTime);

    if (HalCalibration_NrOfTasks == 0)
    {
        // Adding the first task - initialize temperature and next-check time.
#ifdef GP_COMP_HALCORTEXM4
        lastMeasuredTemperature = halADC_MeasureTemperature();
#else
        lastMeasuredTemperature = 0;
#endif
        HalCalibration_NextCheckTime = currentTime + GP_HAL_CALIBRATION_CHECK_INTERVAL_US;
    }

    taskId = HalCalibration_NrOfTasks;
    HalCalibration_TaskInfo[taskId].task = *pTask;
    HalCalibration_TaskInfo[taskId].cbHandler = cbHandler;
    HalCalibration_TaskInfo[taskId].nextCalibrationTime = currentTime + pTask->calibrationPeriod;
    HalCalibration_TaskInfo[taskId].lastCalibrationTemperature = lastMeasuredTemperature;
    HalCalibration_NrOfTasks++;

    return taskId;
}

void gpHal_CalibrationHandleTasks(void)
{
    UInt32 currentTime;
    if (HalCalibration_NrOfTasks == 0)
    {
        // Nothing to do.
        return;
    }

    /* perform any time based calibration
     *  - if time exceeded next calibration check interval OR
     *  - First Calibration routine call after chip wakeup
     */

    HAL_TIMER_GET_CURRENT_TIME_1US(currentTime);
    if (!GP_SCHED_TIME_COMPARE_LOWER_US(currentTime, HalCalibration_NextCheckTime)
            || gpHal_CalibrationGetFirstAfterWakeup())
    {
        // Set time for next check.
        HalCalibration_NextCheckTime = currentTime + GP_HAL_CALIBRATION_CHECK_INTERVAL_US;
        Hal_TimeBasedCalibration();
    }

    /* perform temperature based calibrations */
    HAL_TIMER_GET_CURRENT_TIME_1US(currentTime);
    if (!GP_SCHED_TIME_COMPARE_LOWER_US(currentTime, HalCalibrationTempBased_NextCheckTime))
    {
        // Set time for next check.
        HalCalibrationTempBased_NextCheckTime = currentTime + GP_HAL_CALIBRATION_TEMPERATURE_BASED_CHECK_INTERVAL_US;
        Hal_TemperatureBasedCalibration();
    }
}
