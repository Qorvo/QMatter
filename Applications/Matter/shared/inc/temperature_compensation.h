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

/** @file "temperature_compensation.h"
 *
 *
 *
 *  Implementation of VDD Ram tuning
 */

#ifndef _TEMPERATURE_COMPENSATION_H_
#define _TEMPERATURE_COMPENSATION_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Requests

/** @brief Initialse hardware check
 */
void Application_TemperatureCompensation_Init(void);

/*
 *
 * This API is to enable a wakeup of the chip at a periodic rate. The reason
 * for this wakeup is to enable the gpHal_VddRamTuneTrimAlgorithm which adjusts
 * the PMUD_PMU_VDDRAM_TUNE parameter. This paramater compensates the VDDRAM
 * value for temperature changes.
 * The wakeup is scheduled every 10 seconds.
 * 125*C applications are expected to have a heat-up curve of maximum
 * 0.1*C/second. The mechanism gets triggered at every possiblle 1*C difference.
 *
 */
void Application_TemperatureCompensation_VDDRamTuneWakeup(void);

// Indications

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_TEMPERATURE_COMPENSATION_H_
