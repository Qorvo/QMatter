/*
 * Copyright (c) 2019-2022, Qorvo Inc
 *
 * gpHal_TrimXtal32M.c
 *   This file contains the implementation of 32 MHz Crystal trimming algorithm
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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// #define GP_LOCAL_LOG
#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

#include "gpAssert.h"
#include "gpLog.h"

#include "gpHal.h"
#include "gpSched.h"
#include "gpUtils.h"
#include "gpHal_Calibration.h"
#include "gpHal_kx_regprop_basic.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_HAL_TRIMCAP_ZONE_COUNT 7

/** @brief Configuration of Calibration parameter limits*/
#define HAL_TRIMXTAL32M_MIN_TEMP (-40)
#define HAL_TRIMXTAL32M_MAX_TEMP 125
#define HAL_TRIMXTAL32M_MIN_SETTING 0
#define HAL_TRIMXTAL32M_MAX_SETTING 3

// Check configuration of BSP parameters for 32MHz trim cap calibration.

#if   (GP_BSP_TRIMCAP_ZONE1_SETTING < HAL_TRIMXTAL32M_MIN_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE2_SETTING < HAL_TRIMXTAL32M_MIN_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE3_SETTING < HAL_TRIMXTAL32M_MIN_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE4_SETTING < HAL_TRIMXTAL32M_MIN_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE5_SETTING < HAL_TRIMXTAL32M_MIN_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE6_SETTING < HAL_TRIMXTAL32M_MIN_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE7_SETTING < HAL_TRIMXTAL32M_MIN_SETTING)\

    ||(GP_BSP_TRIMCAP_ZONE1_SETTING > HAL_TRIMXTAL32M_MAX_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE2_SETTING > HAL_TRIMXTAL32M_MAX_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE3_SETTING > HAL_TRIMXTAL32M_MAX_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE4_SETTING > HAL_TRIMXTAL32M_MAX_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE5_SETTING > HAL_TRIMXTAL32M_MAX_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE6_SETTING > HAL_TRIMXTAL32M_MAX_SETTING)\
    ||(GP_BSP_TRIMCAP_ZONE7_SETTING > HAL_TRIMXTAL32M_MAX_SETTING)
        #error GP_BSP_TRIMCAP_ZONE<x>_SETTING needs to be between HAL_TRIMXTAL32M_MIN_SETTING <-> HAL_TRIMXTAL32M_MAX_SETTING
#endif

#if((125 - GP_BSP_TRIMCAP_ZONE6_MAX_TEMP) < GP_BSP_TRIMCAP_HYSTERESIS)
#error GP_BSP_TRIMCAP_ZONE6_MAX_TEMP needs to be lower than the 125*C - GP_BSP_TRIMCAP_HYSTERESIS
#endif

#if ((  GP_BSP_TRIMCAP_ZONE2_MAX_TEMP - GP_BSP_TRIMCAP_ZONE1_MAX_TEMP ) < GP_BSP_TRIMCAP_HYSTERESIS)\
    ||((GP_BSP_TRIMCAP_ZONE3_MAX_TEMP - GP_BSP_TRIMCAP_ZONE2_MAX_TEMP ) < GP_BSP_TRIMCAP_HYSTERESIS)\
    ||((GP_BSP_TRIMCAP_ZONE4_MAX_TEMP - GP_BSP_TRIMCAP_ZONE3_MAX_TEMP ) < GP_BSP_TRIMCAP_HYSTERESIS)\
    ||((GP_BSP_TRIMCAP_ZONE5_MAX_TEMP - GP_BSP_TRIMCAP_ZONE4_MAX_TEMP ) < GP_BSP_TRIMCAP_HYSTERESIS)\
    ||((GP_BSP_TRIMCAP_ZONE6_MAX_TEMP - GP_BSP_TRIMCAP_ZONE5_MAX_TEMP ) < GP_BSP_TRIMCAP_HYSTERESIS)\
    ||((GP_BSP_TRIMCAP_ZONE3_MAX_TEMP - GP_BSP_TRIMCAP_ZONE2_MAX_TEMP ) < GP_BSP_TRIMCAP_HYSTERESIS)
        #error (GP_BSP_TRIMCAP_ZONE<x+1>_MAX_TEMP - GP_BSP_TRIMCAP_ZONE<x>_MAX_TEMP) needs to be greater than GP_BSP_TRIMCAP_HYSTERESIS
#endif


#if   (GP_BSP_TRIMCAP_ZONE1_MAX_TEMP < HAL_TRIMXTAL32M_MIN_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE2_MAX_TEMP < HAL_TRIMXTAL32M_MIN_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE3_MAX_TEMP < HAL_TRIMXTAL32M_MIN_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE4_MAX_TEMP < HAL_TRIMXTAL32M_MIN_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE5_MAX_TEMP < HAL_TRIMXTAL32M_MIN_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE6_MAX_TEMP < HAL_TRIMXTAL32M_MIN_TEMP)\

    ||(GP_BSP_TRIMCAP_ZONE1_MAX_TEMP > HAL_TRIMXTAL32M_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE2_MAX_TEMP > HAL_TRIMXTAL32M_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE3_MAX_TEMP > HAL_TRIMXTAL32M_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE4_MAX_TEMP > HAL_TRIMXTAL32M_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE5_MAX_TEMP > HAL_TRIMXTAL32M_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE6_MAX_TEMP > HAL_TRIMXTAL32M_MAX_TEMP)
        #error GP_BSP_TRIMCAP_ZONE<x>_MAX_TEMP needs to be between HAL_TRIMXTAL32M_MIN_TEMP <-> HAL_TRIMXTAL32M_MAX_TEMP
#endif

#if   (GP_BSP_TRIMCAP_ZONE1_MAX_TEMP > GP_BSP_TRIMCAP_ZONE2_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE2_MAX_TEMP > GP_BSP_TRIMCAP_ZONE3_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE3_MAX_TEMP > GP_BSP_TRIMCAP_ZONE4_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE4_MAX_TEMP > GP_BSP_TRIMCAP_ZONE5_MAX_TEMP)\
    ||(GP_BSP_TRIMCAP_ZONE5_MAX_TEMP > GP_BSP_TRIMCAP_ZONE6_MAX_TEMP)
        #error GP_BSP_TRIMCAP_ZONE<x>_MAX_TEMP needs to be lower than GP_BSP_TRIMCAP_ZONE<x+1>_MAX_TEMP
#endif

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/** @brief 32MHz trim algorithm configuration descriptor*/
typedef struct
{
    /** @brief hysteresis amount in delta Â°C that will trigger new calibration of Â°C  */
    UInt8 hysteresis;

    /** @brief max temperature limits zone is split from next zone [-39-124Â°C]*/
    Int8 zoneMaxTemps[GP_HAL_TRIMCAP_ZONE_COUNT-1];

    /** @brief Setting for trim capacitance register corresponding to temperature zone */
    UInt8 zoneSettings[GP_HAL_TRIMCAP_ZONE_COUNT];

} gpHal_32M_trim_settings_t;


/******************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static const gpHal_32M_trim_settings_t trimconfig =
{
    //
    .hysteresis = GP_BSP_TRIMCAP_HYSTERESIS,
    .zoneMaxTemps =
        {
        GP_BSP_TRIMCAP_ZONE1_MAX_TEMP,
        GP_BSP_TRIMCAP_ZONE2_MAX_TEMP,
        GP_BSP_TRIMCAP_ZONE3_MAX_TEMP,
        GP_BSP_TRIMCAP_ZONE4_MAX_TEMP,
        GP_BSP_TRIMCAP_ZONE5_MAX_TEMP,
        GP_BSP_TRIMCAP_ZONE6_MAX_TEMP
        },
    .zoneSettings =
        {
        GP_BSP_TRIMCAP_ZONE1_SETTING,
        GP_BSP_TRIMCAP_ZONE2_SETTING,
        GP_BSP_TRIMCAP_ZONE3_SETTING,
        GP_BSP_TRIMCAP_ZONE4_SETTING,
        GP_BSP_TRIMCAP_ZONE5_SETTING,
        GP_BSP_TRIMCAP_ZONE6_SETTING,
        GP_BSP_TRIMCAP_ZONE7_SETTING
        },
};

/* Process data */
static UInt8 trimcapSetPoint;

/*****************************************************************************
*                   Static Function Definitions
******************************************************************************/
static void gpHal_Xtal32MHzTrimAlgorithmApplyTrimSettings(void);

static void gpHal_Xtal32MHzTrimAlgorithmUpdateAlgoTrimSettings(const gpHal_CalibrationTask_t* task)
{
    Int8 temperature;
    UInt8 currentTrimcapSetting = GP_WB_READ_PMUD_XO_TRIMCAP();
    temperature = Q_PRECISION_DECR8(task->temperature);
    // Clip on min and max temperatures
    temperature = clamp(temperature, HAL_TRIMXTAL32M_MIN_TEMP, HAL_TRIMXTAL32M_MAX_TEMP);

    // Binary search, start from middle as this is usually largest zone
    // and close to room temperature. The size for k8c with iar compiler
    // is the same as for loop about same but it will determine the correct zone faster.
    if (temperature > trimconfig.zoneMaxTemps[3])
    {
        if (temperature <= trimconfig.zoneMaxTemps[4])
        {
            trimcapSetPoint = trimconfig.zoneSettings[4];
            GP_LOG_PRINTF("TrimXtal32M:%d*C => Zone5 ", 0, temperature);
        }
        else
        {
            if (temperature <= trimconfig.zoneMaxTemps[5])
            {
                trimcapSetPoint = trimconfig.zoneSettings[5];
                GP_LOG_PRINTF("TrimXtal32M:%d*C => Zone6 ", 0, temperature);
            }
            else
            {
                trimcapSetPoint = trimconfig.zoneSettings[6];
                GP_LOG_PRINTF("TrimXtal32M:%d*C => Zone7 ", 0, temperature);
            }
        }
    }
    else
    {
        if (temperature > trimconfig.zoneMaxTemps[2])
        {
            trimcapSetPoint = trimconfig.zoneSettings[3];
            GP_LOG_PRINTF("TrimXtal32M:%d*C => Zone4 ", 0, temperature);
        }
        else
        {
            if (temperature > trimconfig.zoneMaxTemps[1])
            {
                trimcapSetPoint = trimconfig.zoneSettings[2];
                GP_LOG_PRINTF("TrimXtal32M:%d*C => Zone3 ", 0, temperature);
            }
            else
            {
                if (temperature > trimconfig.zoneMaxTemps[0])
                {
                    trimcapSetPoint = trimconfig.zoneSettings[1];
                    GP_LOG_PRINTF("TrimXtal32M:%d*C => Zone2 ", 0, temperature);
                }
                else
                {
                    trimcapSetPoint = trimconfig.zoneSettings[0];
                    GP_LOG_PRINTF("TrimXtal32M:%d*C => Zone1 ", 0, temperature);
                }
            }
        }
    }
    GP_LOG_SYSTEM_PRINTF("[%d*C] xotrimcap goal:%d cur:%d", 0, temperature, trimcapSetPoint, currentTrimcapSetting);

    while (gpSched_ExistsEvent(gpHal_Xtal32MHzTrimAlgorithmApplyTrimSettings))
    {
        gpSched_UnscheduleEvent(gpHal_Xtal32MHzTrimAlgorithmApplyTrimSettings);
    }

    // if settings changed, call gpHal_Xtal32MHzTrimAlgorithmApplyTrimSettings()
    if (currentTrimcapSetting != trimcapSetPoint)
    {
        gpHal_Xtal32MHzTrimAlgorithmApplyTrimSettings();
    }
}

static void gpHal_Xtal32MHzTrimAlgorithmApplyTrimSettings(void)
{

    /* Continuous Adjustment of Trim Capacitors */
    UInt8 currentTrimcapSetting = GP_WB_READ_PMUD_XO_TRIMCAP();
    UInt8 nextTrimcapSetting = currentTrimcapSetting;

    UInt8 backupAlwaysUseFastClockAsSource = GP_WB_READ_STANDBY_ALWAYS_USE_FAST_CLOCK_AS_SOURCE();
    UInt8 backupPrescaleUcCore = GP_WB_READ_STANDBY_PRESCALE_UCCORE();

    // If current trim setting equals the previous trim setting, ignore the adjustment
    if (currentTrimcapSetting == trimcapSetPoint)
    {
        return ;
    }

    GP_WB_WRITE_STANDBY_ALWAYS_USE_FAST_CLOCK_AS_SOURCE(0);
    // If clockspeed is 64 MHz, play it safe, and switch back to 32 MHz
    if (backupPrescaleUcCore == GP_WB_ENUM_CLOCK_SPEED_M64)
    {
        GP_WB_WRITE_STANDBY_PRESCALE_UCCORE(GP_WB_ENUM_CLOCK_SPEED_M32);
    }

    // Determine direction in which to trim
    if (currentTrimcapSetting < trimcapSetPoint)
    {
        nextTrimcapSetting = currentTrimcapSetting + 1;
    }
    else if (currentTrimcapSetting > trimcapSetPoint)
    {
        nextTrimcapSetting = currentTrimcapSetting - 1;
    }

    GP_LOG_SYSTEM_PRINTF("[xoTrim32Mhz]%d->%dgoal:%d", 0, currentTrimcapSetting, nextTrimcapSetting, trimcapSetPoint);
    GP_WB_WRITE_PMUD_XO_TRIMCAP(nextTrimcapSetting);

    // Restore clockspeed settings
    GP_WB_WRITE_STANDBY_ALWAYS_USE_FAST_CLOCK_AS_SOURCE(backupAlwaysUseFastClockAsSource);
    if (backupPrescaleUcCore == GP_WB_ENUM_CLOCK_SPEED_M64)
    {
        GP_WB_WRITE_STANDBY_PRESCALE_UCCORE(GP_WB_ENUM_CLOCK_SPEED_M64);
    }

    // if not yet fully tuned, schedule gpHal_Xtal32MHzTrimAlgorithmApplyTrimSettings() to do continuous adjustment
    if (GP_WB_READ_PMUD_XO_TRIMCAP() != trimcapSetPoint)
    {
        /* schedule events to continue tuning */
        gpSched_ScheduleEvent(10000, gpHal_Xtal32MHzTrimAlgorithmApplyTrimSettings);
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpHal_Xtal32MHzTrimAlgorithmInit(void)
{

    hal_InitADC();

    // Update trim settings whenever temperature difference exceeds config.hysterisis
    gpHal_CalibrationTask_t calTask;
    UInt8 calTaskHandle;
    MEMSET(&calTask, 0, sizeof(gpHal_CalibrationTask_t));
    calTask.flags = GP_HAL_CALIBRATION_FLAG_TEMPERATURE_SENSITIVE;
    calTask.temperatureThreshold = UQ_PRECISION_INCR8(trimconfig.hysteresis);
    GP_LOG_SYSTEM_PRINTF("TrimXtal32M hys=%d",0, trimconfig.hysteresis);
    calTaskHandle = gpHal_CalibrationCreateTask(&calTask, gpHal_Xtal32MHzTrimAlgorithmUpdateAlgoTrimSettings);
    // Updating setpoint so it doesn't get stuck on steady-state hysteresis
    gpHal_Xtal32MHzTrimAlgorithmUpdateAlgoTrimSettings(&calTask);
    GP_ASSERT_DEV_EXT(calTaskHandle != GP_HAL_CALIBRATION_INVALID_TASK_HANDLE);
}
