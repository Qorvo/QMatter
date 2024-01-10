/*
 * Copyright (c) 2023, Qorvo Inc
 *
 * gpHal_TrimRxMode.c
 *
 * This file contains algorithm that changes rx mode based on temperature
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
 *
 */

#include "hal.h"
#include "gpHal.h"
#include "global.h"
#include "gpHal_DEFS.h"
#include "gpSched.h"
#include "gpHal_Calibration.h"
#include "gpRadio.h"
#include "gpHal_TrimRxMode.h"

#define GP_COMPONENT_ID                             (GP_COMPONENT_ID_GPHAL)
#define GPHAL_RX_MODE_TEMPERATURE_THESHOLD          (-25)
#define GPHAL_RX_MODE_TEMPERATURE_DELTA             (5)

static Bool lastTemperatureLow;

static void gpHal_RxModeSwitchAlgoRun(const gpHal_CalibrationTask_t* task);
static void gpHal_SetLowTemperature(Bool lowTemp);

static void gpHal_RxModeSwitchAlgoRun(const gpHal_CalibrationTask_t* task)
{
    Int8 temperature = Q_PRECISION_DECR8(task->temperature);

    GP_LOG_PRINTF("TrimRxMode: temperature = %d", 0, temperature);
    if (temperature  >= GPHAL_RX_MODE_TEMPERATURE_THESHOLD)
    {
        if(lastTemperatureLow)
        {
            gpHal_SetLowTemperature(false);
        }
    }
    else if (temperature < GPHAL_RX_MODE_TEMPERATURE_THESHOLD)
    {
        if(!lastTemperatureLow)
        {
            gpHal_SetLowTemperature(true);
        }
    }
}

static void gpHal_SetLowTemperature(Bool lowTemp)
{
    static Bool hiTempMultiStandard, hiTempMultiChannel, hiTempHighSensitivity;

    lastTemperatureLow = lowTemp;

    if(lowTemp)
    {
        // Store settings from high temperature mode:
        gpRadio_GetRxMode(&hiTempMultiStandard, &hiTempMultiChannel, &hiTempHighSensitivity);

        // HighSensitivity mode already enabled:
        if(hiTempHighSensitivity)
        {
            return;
        }

        // Enable the HighSensitivity mode.
        // HighSensitivity mode is available only when MultiStandard and MultiChannel features are disabled:
        if(gpRadio_SetRxMode(hiTempMultiStandard, hiTempMultiChannel, true) == gpRadio_StatusSuccess)
        {
            GP_LOG_SYSTEM_PRINTF(
                "Radio configuration optimized for low temperature operation in 802.15.4 - power consumption increased",
                0);
        }
    }
    else
    {
        // Restore settings from high temperature mode:
        gpRadio_SetRxMode(hiTempMultiStandard, hiTempMultiChannel, hiTempHighSensitivity);
    }
}

void gpHal_RxModeSwitchAlgoInit(void)
{
    lastTemperatureLow = false;
    // create a calibration task to trigger at every 5C to check if temperature exceeded threshold
    // and correct RxMode
    gpHal_CalibrationTask_t calTask;
    UInt8 calTaskHandle;
    MEMSET(&calTask, 0, sizeof(gpHal_CalibrationTask_t));
    calTask.flags = GP_HAL_CALIBRATION_FLAG_TEMPERATURE_SENSITIVE | \
                    GP_HAL_CALIBRATION_FLAG_CALIBRATE_ON_CHIP_WAKEUP | \
                    GP_HAL_CALIBRATION_FLAG_CALIBRATE_ON_CALIB_TASK_CREATION;
    calTask.temperatureThreshold = UQ_PRECISION_INCR8(GPHAL_RX_MODE_TEMPERATURE_DELTA);
    calTaskHandle = gpHal_CalibrationCreateTask(&calTask, gpHal_RxModeSwitchAlgoRun);
    GP_ASSERT_DEV_EXT(calTaskHandle != GP_HAL_CALIBRATION_INVALID_TASK_HANDLE);
}

Bool gpHal_IsLowTemp(void)
{
    return lastTemperatureLow;
}
