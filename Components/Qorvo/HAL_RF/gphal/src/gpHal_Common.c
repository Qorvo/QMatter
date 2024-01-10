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
 * Common code that can be shared over all gpHal flavors
 *
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG
#include "gpHal.h"
#include "gpHal_DEFS.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_GPHAL

#define GPHAL_BLE_PREAMBLE_BYTE_ODD         0x55
#define GPHAL_BLE_PREAMBLE_BYTE_EVEN        0xAA
#define GPHAL_BLE_PREAMBLE_BYTE_CODED       0x3C

/*****************************************************************************
 *                   Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/******************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

UInt8 gpHal_GetPreambleSymbol(gpHal_BleTxPhy_t phy, UInt32 accessAddress)
{
#ifdef GP_HAL_DIVERSITY_BLE_LONG_RANGE_SUPPORTED
    if(phy >= gpHal_BleTxPhyCoded125kb)
    {
         return GPHAL_BLE_PREAMBLE_BYTE_CODED;
    }
    else
#endif //GP_HAL_DIVERSITY_BLE_LONG_RANGE_SUPPORTED
    {
        if(accessAddress & 0x01)
        {
            return GPHAL_BLE_PREAMBLE_BYTE_ODD;
        }
        else
        {
            return GPHAL_BLE_PREAMBLE_BYTE_EVEN;
        }
    }
}
