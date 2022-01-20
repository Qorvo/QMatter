/*
 * Copyright (c) 2017, 2020, Qorvo Inc
 *
 * misc_qorvo.c
 *   This file contains the implementation of the qorvo misc api for openthread.
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
 * $Header: //depot/release/Embedded/Applications/P959_OpenThread/v1.1.23.1/comps/qvOT/src/misc_qorvo.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

#define GP_COMPONENT_ID GP_COMPONENT_ID_QVOT

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpReset.h"
#include "gpSched.h"

#ifdef GP_COMP_GPHAL
#include "hal_user_license.h"
#endif


#include "misc_qorvo.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define QORVO_RESET_DELAY_US (1 * 1000) // 100ms

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/



/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
bool gPlatformPseudoResetWasRequested;

#if !defined(HAL_DIVERSITY_USB)
static void delayedReset(void)
{
    gpReset_ResetSystem();
}
#endif

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void qorvoPlatReset(void)
{
    // short delay for the reset to get the ACKs out.
#if defined(HAL_DIVERSITY_USB)
    gPlatformPseudoResetWasRequested = true;
#else
    gpSched_ScheduleEvent(0, delayedReset);
#endif
}


void qorvoGetUserLicense(void)
{
    /* prevent the linker from throwing away the user license */
#ifdef GP_COMP_GPHAL
    hal_get_user_license();
#endif
}

