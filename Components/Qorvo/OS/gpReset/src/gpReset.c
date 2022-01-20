/*
 * Copyright (c) 2011-2014, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
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
 * $Header: //depot/release/Embedded/Components/Qorvo/OS/v2.10.2.1/comps/gpReset/src/gpReset.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_RESET

#include "gpReset.h"
#include "hal.h"
#ifdef GP_DIVERSITY_NVM
#include "gpNvm.h"
#endif //GP_DIVERSITY_NVM
#include "gpLog.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define RESET_REASON_MAX        5
/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/
#define RESET_HAL_TO_GPRESET_REASON(mapping, halReason)              ((halReason < RESET_REASON_MAX)? mapping[halReason] : gpReset_ResetReason_UnSpecified)
/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
gpReset_ResetReason_t gpReset_Reason;
#ifdef GP_DIVERSITY_NVM
#define NVM_TAG_RESET_REASON    0
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
const gpNvm_IdentifiableTag_t ROM gpReset_NvmElements[] FLASH_PROGMEM = {
    {(UInt16)(GP_COMPONENT_ID<<8), &gpReset_Reason , sizeof(gpReset_ResetReason_t), gpNvm_UpdateFrequencyInitOnly, NULL, NULL}
};
#else /* GP_NVM_DIVERSITY_ELEMENT_IF */
const gpNvm_Tag_t ROM gpReset_NvmSection[] FLASH_PROGMEM = {
    {&gpReset_Reason , sizeof(gpReset_ResetReason_t), gpNvm_UpdateFrequencyInitOnly, NULL}
};
#endif /* GP_NVM_DIVERSITY_ELEMENT_IF */
#endif //GP_DIVERSITY_NVM

/*****************************************************************************
 *                    External Data Definition
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpReset_Init(void)
{
    // local mapping from hal reset reasons to gpReset reset reasons
    gpReset_ResetReason_t resetReasonMapping[RESET_REASON_MAX] = {
            gpReset_ResetReason_UnSpecified, gpReset_ResetReason_HW_Por, gpReset_ResetReason_SW_Por,
            gpReset_ResetReason_HW_BrownOutDetected, gpReset_ResetReason_HW_Watchdog
    };

#ifdef GP_DIVERSITY_NVM
#ifdef GP_NVM_DIVERSITY_ELEMENT_IF
    gpNvm_RegisterElements(gpReset_NvmElements, sizeof(gpReset_NvmElements)/sizeof(gpNvm_IdentifiableTag_t));
#else /* GP_NVM_DIVERSITY_ELEMENT_IF */
    gpNvm_RegisterSection(GP_COMPONENT_ID, gpReset_NvmSection, number_of_elements(gpReset_NvmSection), NULL);
#endif /* GP_NVM_DIVERSITY_ELEMENT_IF */
#endif //GP_DIVERSITY_NVM


    // get hal reset reason and convert to gpReset_ResetReason
    gpReset_Reason = RESET_HAL_TO_GPRESET_REASON(resetReasonMapping, HAL_GET_RESET_REASON());
}

void gpReset_ResetBySwPor(void)
{
    HAL_RESET_UC();
}

void gpReset_ResetByWatchdog(void)
{
    HAL_WDT_FORCE_TRIGGER();
}

gpReset_ResetReason_t gpReset_GetResetReason(void)
{
    return gpReset_Reason;
}

