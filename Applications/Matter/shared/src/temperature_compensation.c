/*
 * Copyright (c) 2022, Qorvo Inc
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
 */

/** @file "temperature_compensation.c"
 *
 *  Application API
 *
 *  Implementation of VDD RAM tuning procedure
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

#include "global.h"
#include "gpLog.h"
#include "gpSched.h"

#include "temperature_compensation.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define DELAY_10_SECONDS (10 * 1000 * 1000)
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

/** @brief Initialse hardware check
 */
void Application_TemperatureCompensation_Init(void)
{
    gpSched_ScheduleEvent(DELAY_10_SECONDS, Application_TemperatureCompensation_VDDRamTuneWakeup);
}

/*
 *
 * This API is to enable a wakeup of the chip at a periodic rate. The reason
 * for this wakeup is to enable the gpHal_VddRamTuneTrimAlgorithm which adjusts
 * the PMUD_PMU_VDDRAM_TUNE parameter. This parameter compensates the VDDRAM
 * value for temperature changes.
 * The wakeup is scheduled every 10 seconds.
 * 125*C applications are expected to have a heat-up curve of maximum
 * 0.1*C/second. The mechanism gets triggered at every possible 1*C difference.
 *
 */
void Application_TemperatureCompensation_VDDRamTuneWakeup(void)
{
    GP_LOG_PRINTF("wkup->vddRamTune", 0);
    gpSched_ScheduleEvent(DELAY_10_SECONDS, Application_TemperatureCompensation_VDDRamTuneWakeup);
}
