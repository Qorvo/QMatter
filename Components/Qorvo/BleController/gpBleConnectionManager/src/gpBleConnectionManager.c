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

// #define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLECONNECTIONMANAGER

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBleConnectionManager.h"
#include "gpBleLlcp.h"
#include "gpLog.h"

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
    gpHci_ConnectionHandle_t connectionHandleCounter;
} Ble_ConnectionContext_t;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/


static gpHci_Result_t BleConnectionManager_DisconnectAction(gpHci_DisconnectCommand_t* pDisconnect);
static Bool BleConnectionManager_IsHandleInUse(gpHci_ConnectionHandle_t connectionHandle);
static void BleConnectionManager_IncrementConnectionHandle(void);
static void BleConnectionManager_SendDisconnectEvent(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t reason);

// Callbacks regarding connections that will be registered to gpBle
static gpHci_ConnectionHandle_t BleConnectionManager_AllocateHciConnectionHandle(void);
static void BleConnectionManager_EstablishConnection(gpHci_ConnectionHandle_t hciHandle, gpHci_Result_t result);
static void BleConnectionManager_DisconnectAcl(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t reason);
static void BleConnectionManager_DisconnectConnection(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t reason);

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static Ble_ConnectionContext_t Ble_ConnectionContext;

static const gpBle_ConnectionCallbacks_t ConnectionManager_Callbacks =
{
    .cbAllocateHandle = BleConnectionManager_AllocateHciConnectionHandle,
    .cbEstablish = BleConnectionManager_EstablishConnection,
    .cbDisconnect = BleConnectionManager_DisconnectConnection,
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/


gpHci_Result_t BleConnectionManager_DisconnectAction(gpHci_DisconnectCommand_t* pDisconnect)
{
    if(gpBleLlcp_IsAclHandleInUse(pDisconnect->connectionHandle))
    {
        return gpBleLlcp_DisconnectAcl(pDisconnect->connectionHandle, pDisconnect->reason);
    }

    else
    {
        return gpHci_ResultUnknownConnectionIdentifier;
    }
}

Bool BleConnectionManager_IsHandleInUse(gpHci_ConnectionHandle_t connectionHandle)
{
    if(gpBleLlcp_IsAclHandleInUse(connectionHandle))
    {
        return true;
    }


    return false;
}

void BleConnectionManager_IncrementConnectionHandle(void)
{
    Ble_ConnectionContext.connectionHandleCounter++;

    if (Ble_ConnectionContext.connectionHandleCounter == (GPBLE_CONNECTION_HANDLE_MAX + 1))
    {
        Ble_ConnectionContext.connectionHandleCounter = 0;
    }
}

void BleConnectionManager_SendDisconnectEvent(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t reason)
{
    // Notify the host if needed
    gpHci_EventCbPayload_t params;

    MEMSET(&params, 0x00, sizeof(params));
    params.disconnectCompleteParams.status = gpHci_ResultSuccess;   // Only relevant value for disconnect complete
    params.disconnectCompleteParams.connectionHandle = connHandle;
    params.disconnectCompleteParams.reason = reason;

    gpBle_ScheduleEvent(0, gpHci_EventCode_DisconnectionComplete, &params);
}

gpHci_ConnectionHandle_t BleConnectionManager_AllocateHciConnectionHandle(void)
{
    gpHci_ConnectionHandle_t hciHandle;

    while (BleConnectionManager_IsHandleInUse(Ble_ConnectionContext.connectionHandleCounter))
    {
        BleConnectionManager_IncrementConnectionHandle();
    }

    hciHandle = Ble_ConnectionContext.connectionHandleCounter;

    // Prepare new hci handle for next connection
    BleConnectionManager_IncrementConnectionHandle();

    return hciHandle;
}

void BleConnectionManager_EstablishConnection(gpHci_ConnectionHandle_t hciHandle, gpHci_Result_t result)
{
    // Nothing to be done for ACL connections at the moment

}

void BleConnectionManager_DisconnectAcl(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t reason)
{

    gpBleLlcp_AclConnectionStop(gpBleLlcp_HciHandleToIntHandle(connHandle), reason);

    BleConnectionManager_SendDisconnectEvent(connHandle, reason);
}


void BleConnectionManager_DisconnectConnection(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t reason)
{
    if(gpBleLlcp_IsAclHandleInUse(connHandle))
    {
        BleConnectionManager_DisconnectAcl(connHandle, reason);
    }
    else
    {
        // Unknown connection handle
        GP_ASSERT_DEV_INT(false);
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleConnectionManager_Init(void)
{
    gpBle_RegisterConnectionCallbacks(&ConnectionManager_Callbacks);
}

void gpBleConnectionManager_Reset(Bool firstReset)
{
    Ble_ConnectionContext.connectionHandleCounter = 0;

}

/*****************************************************************************
 *                    Service Function Definitions
 *****************************************************************************/


gpHci_Result_t gpBle_Disconnect(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_STATUS(pEventBuf->eventCode);

    return BleConnectionManager_DisconnectAction(&pParams->Disconnect);
}
