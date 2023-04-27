/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
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

#include "gpJumpTables.h"
#include "gpBle.h"
#include "gpBle_defs.h"
#include "gpBleAdvertiser.h"
#include "gpBleAdvertiser_defs.h"
#include "gpLog.h"

/*****************************************************************************
 *                    NRT ROM patch fix version numbers
 *****************************************************************************/

/*****************************************************************************
 *                    ROM Function prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEADVERTISER

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
/*****************************************************************************
 *                    Tmp extern Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

gpHci_Result_t Ble_SetAdvertiseChecker(UInt8 advertisingEnable)
{
    if(GP_BLEADVERTISER_GET_GLOBALS()->Ble_AdvertisingAttributes_ptr->advertisingEnabled && advertisingEnable)
    {
        GP_LOG_PRINTF("Advertising already enabled ",0);
        return gpHci_ResultSuccess; // SDP004-2343
    }

    return gpHci_ResultSuccess;
}

/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/
