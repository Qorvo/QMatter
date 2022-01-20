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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleActivityManager/src/gpBleActivityManager_defs.h#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */


/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpBleActivityManager.h"

#if defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
#include "gpBleAdvertiser.h"
#endif //GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_SLAVE

#if defined(GP_DIVERSITY_BLE_OBSERVER) || defined(GP_DIVERSITY_BLE_MASTER)
#include "gpBleScanner.h"
#endif //GP_DIVERSITY_BLE_OBSERVER || GP_DIVERSITY_BLE_MASTER

#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
#include "gpBleInitiator.h"
#include "gpBleLlcp.h"
#endif //GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    Ble_ActivityId_t activityId;
    // Only used for connections
    UInt8 slaveNbrConsecutiveLargeCorrWindows;
    Bool connectionEstablished;
} Ble_MonitorLinkContext_t;

typedef struct {
    UInt32 accessAddress;
    UInt32 initialCorrelationTs;
    Ble_IntConnId_t connId;
    Bool terminationOngoing;
    gpHci_ConnectionHandle_t hciHandle;
    UInt16 timeoutUnit;
    UInt16 intervalUnit;
    Bool centralConnection;
} BleActivityManager_EventProcessed_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    External Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleActivityManager_InitCommon(void);
void gpBleActivityManager_ResetCommon(void);
void gpBleActivityManager_OnCloseConnectionCommon(Ble_IntConnId_t connId);
Bool BleActivityManager_GetMonitorContext(Ble_IntConnId_t connId, Ble_MonitorLinkContext_t* pMonitorContext);
void BleActivityManager_UpdateMonitorContext(Ble_MonitorLinkContext_t* pMonitorContext);
void gpBleActivityManager_ConnectionEventDoneCommon(BleActivityManager_EventProcessed_t* pContext, gpHal_ConnEventMetrics_t* pMetrics);
void gpBleActivityManager_GetEventProcessedContext(gpHci_ConnectionHandle_t hciHandle, BleActivityManager_EventProcessed_t* pEvent);
void gpBleActivityManager_GetCisEventProcessedContext(gpHci_ConnectionHandle_t hciHandle, BleActivityManager_EventProcessed_t* pEvent);
