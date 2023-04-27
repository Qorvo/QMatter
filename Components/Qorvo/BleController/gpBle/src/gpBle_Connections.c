/*
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

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLE

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle_defs.h"
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

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static gpBle_ConnectionCallbacks_t* Ble_ConnectionCallbacks;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBle_ConnectionsInit(void)
{

}

void gpBle_ConnectionsReset(Bool firstReset)
{

}

UInt8 gpBle_GetPreambleSymbol(gpHci_Phy_t phy, gpBle_AccessAddress_t accessAddress)
{
    {
        if(accessAddress & 0x01)
        {
            return GPBLE_PREAMBLE_BYTE_ODD;
        }
        else
        {
            return GPBLE_PREAMBLE_BYTE_EVEN;
        }
    }
}

void gpBle_RegisterConnectionCallbacks(const gpBle_ConnectionCallbacks_t* pCallbacks)
{
    if(pCallbacks == NULL)
    {
        return;
    }

    Ble_ConnectionCallbacks = (gpBle_ConnectionCallbacks_t*) pCallbacks;
}

gpHci_ConnectionHandle_t gpBle_AllocateHciConnectionHandle(void)
{
    if(Ble_ConnectionCallbacks->cbAllocateHandle == NULL)
    {
        GP_ASSERT_DEV_INT(false);

        return GP_HCI_CONNECTION_HANDLE_INVALID;
    }

    return Ble_ConnectionCallbacks->cbAllocateHandle();
}

void gpBle_EstablishConnection(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t result)
{
    if(Ble_ConnectionCallbacks->cbEstablish != NULL)
    {
        Ble_ConnectionCallbacks->cbEstablish(connHandle, result);
    }
}

void gpBle_StopConnection(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t reason)
{
    if(Ble_ConnectionCallbacks->cbDisconnect != NULL)
    {
        Ble_ConnectionCallbacks->cbDisconnect(connHandle, reason);
    }
}

void gpBle_SendHciNumberOfCompletedPacketsEvent(gpHci_ConnectionHandle_t connHandle)
{
    gpBle_EventBuffer_t* pEventBuf;
    pEventBuf = gpBle_AllocateEventBuffer(Ble_EventBufferType_Unsolicited);
    GP_ASSERT_DEV_EXT(pEventBuf != NULL); // not returning a NoCP is a fatal error
    pEventBuf->eventCode = gpHci_EventCode_NumberOfCompletedPackets;
    pEventBuf->payload.numberOfCompletedPackets.nrOfHandles = 1;
    pEventBuf->payload.numberOfCompletedPackets.nrOfHciPackets = 1;
    pEventBuf->payload.numberOfCompletedPackets.handle = connHandle;
    gpBle_SendEvent(pEventBuf);
}
