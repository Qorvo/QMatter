/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 *   Bluetooth Low Energy interface
 *   Implementation of gpBle
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

//#define GP_LOCAL_LOG
#define GP_COMPONENT_ID GP_COMPONENT_ID_BLE

#include "gpBle.h"
#include "gpBle_defs.h"
#include "gpHci_Includes.h"
#include "gpLog.h"
#include "gpPd.h"
#include "gpSched.h"

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleDataChannelTxQueue.h"
#endif

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
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Function Definitions
 *****************************************************************************/

/*
The data channel Tx queue handler currently gives an indication to LLCP or Data TX block whenever a pbm might be available.

Some things we need to add:
- Implement a BLE Resource Manager, which is basically a small layer above gpPd. Every pd that is claimed in BLE should go through this interface.
  This resource manager should also make it possible to register a callback that is triggered whenever a pd is freed.
- The Ble_xxxResourceAvailableIndications should not have the pdHandle in the interface, but only the connection identifier.
- Decide whether or not we need to schedule the resource available indication or call it directly when a pd is freed (latter gives faster usage of available pds, but is trickier).
*/

void Ble_RMFreeResource(Ble_IntConnId_t connId, gpPd_Handle_t handle)
{
    gpPd_FreePd(handle);

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
    // Always inform the upper layers that a resource became available - even if we don't know where that resource came from
    // The gpBle_cbDataTxQueueResourceAvailable knows what to do with an invalid connId
    gpBle_cbDataTxQueueResourceAvailable(connId);
#endif
}

gpHci_Result_t Ble_RMGetResource(gpPd_Loh_t* pdLoh)
{
    // allocate pd
    pdLoh->handle = gpPd_GetCustomPd(gpPd_BufferTypeBle, GP_HAL_PBM_MAX_SIZE);
    if(gpPd_CheckPdValid(pdLoh->handle) != gpPd_ResultValidHandle)
    {
        GP_LOG_PRINTF("Unable to allocate pd!",0);
        return gpHci_ResultMemoryCapacityExceeded;
    }

    pdLoh->length = 0;
    pdLoh->offset = GP_BLE_ADV_CHANNEL_PDU_MAX_OFFSET;

    return gpHci_ResultSuccess;
}
