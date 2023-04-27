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

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEDATARX

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleDataChannelRxQueue.h"
#include "gpBleDataCommon.h"
#include "gpBleDataRx.h"
#include "gpBleLlcp.h"
#include "gpBle_defs.h"
#include "gpHci_Includes.h"
#include "gpLog.h"
#include "gpSched.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_HCI_HEADER_SIZE 4

#define BLE_DATA_RX_MAX_BUFFER_SIZE (GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_RX + BLE_HCI_HEADER_SIZE)

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct{
    gpHci_ConnectionHandle_t            connHandle;
    gpHci_PacketBoundaryFlag_t          flag;
    Bool                                isOpen; // False if sent to HCI
    UInt16                              length;
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    UInt8*                              data;
#else
    UInt8                               data[BLE_HCI_HEADER_SIZE + GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_RX];
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
} Ble_RxDataBuffer_t;

typedef struct{
    Bool                                DataReceived; // needed for SW-4422, SW-4259, SW-4487
    Ble_cbUnexpectedDataRx_t            cbUnexpectedDataRx;
} Ble_DataRxLinkContext_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static Ble_RxDataBuffer_t Ble_RxDataBuffers[GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX];

#ifdef GP_DIVERSITY_DEVELOPMENT
static Bool gpBle_VsdFlushAtEndOfConnectionEvent;
static gpHci_VsdSinkMode_t gpBle_VsdNullSinkMode;
#endif /* GP_DIVERSITY_DEVELOPMENT */

static Ble_DataRxLinkContext_t DataRxLinkContext[BLE_LLCP_MAX_NR_OF_CONNECTIONS];
Bool flow_stop_active = false;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
void* WsfMsgAlloc(UInt16 length);
void WsfMsgFree(void *pMsg);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

static gpBle_RxBufferHandle_t GetRxDataBufferIdx(gpHci_ConnectionHandle_t connHandle)
{
    gpBle_RxBufferHandle_t bufferIdx = 0;
    // Simple buffer selection mechanism to help reduce the amount of packet transfers
    while (bufferIdx < GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX)
    {
        // if buffer already exists for this connection pick this one
        if (Ble_RxDataBuffers[bufferIdx].isOpen &&
            Ble_RxDataBuffers[bufferIdx].connHandle == connHandle)
        {
            break;
        }
        ++bufferIdx;
    }
    return bufferIdx;
}

static gpBle_RxBufferHandle_t GetEmptyRxDataBufferIdx(void)
{
    gpBle_RxBufferHandle_t bufferIdx = 0;
    while (bufferIdx < GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX)
    {
        if ( (Ble_RxDataBuffers[bufferIdx].length == BLE_HCI_HEADER_SIZE)
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
             && (NULL != Ble_RxDataBuffers[bufferIdx].data) // find a buffer with valid assigned WSF buffer
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
           )
        {
            break;
        }
        ++bufferIdx;
    }
    return bufferIdx;
}

static gpBle_RxBufferHandle_t AllocRxDataBufferIdx(gpHci_ConnectionHandle_t connHandle, gpHci_PacketBoundaryFlag_t flag)
{
    gpBle_RxBufferHandle_t bufferIdx = GetEmptyRxDataBufferIdx();
    if (bufferIdx < GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX)
    {
        Ble_RxDataBuffers[bufferIdx].connHandle = connHandle;
        Ble_RxDataBuffers[bufferIdx].flag = flag;
        Ble_RxDataBuffers[bufferIdx].length = BLE_HCI_HEADER_SIZE;
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
        // WSF bufffer must have been allocated already
        GP_ASSERT_DEV_EXT(NULL != Ble_RxDataBuffers[bufferIdx].data);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
        MEMSET(Ble_RxDataBuffers[bufferIdx].data,0,BLE_HCI_HEADER_SIZE);
    }
    return bufferIdx;
}

static void FreeRxDataBufferIdx(gpBle_RxBufferHandle_t bufferIdx)
{
    Ble_RxDataBuffers[bufferIdx].connHandle = GP_HCI_CONNECTION_HANDLE_INVALID;
    Ble_RxDataBuffers[bufferIdx].flag = gpHci_L2CAP_Invalid;
    Ble_RxDataBuffers[bufferIdx].isOpen = true;
    Ble_RxDataBuffers[bufferIdx].length = BLE_HCI_HEADER_SIZE;
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    // This function is called from gpBle_DataRxReset() and from gpBle_HciDataRxConfirm()
    // When called from gpBle_DataRxReset():
    //     - Upon reset, Ble_RxDataBuffers[bufferIdx].data does not contain a valid WSF buffer pointer
    // When called from gpBle_HciDataRxConfirm()  i.e. during normal operation, in the call-tree under in Ble_FlushRxBuffer()
    //     - The application now owns the WSF buffer containing the previously recevied data
    //       so invalidate Ble_RxDataBuffers[bufferIdx].data
    Ble_RxDataBuffers[bufferIdx].data = NULL;
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
}

static void Ble_FlushRxBuffer(gpBle_RxBufferHandle_t bufferIdx)
{
    Ble_RxDataBuffers[bufferIdx].isOpen = false;
#ifdef GP_DIVERSITY_DEVELOPMENT
    if (gpBle_VsdNullSinkMode == gpHci_VsdSinkMode_Null)
    {
        gpBle_HciDataRxConfirm(bufferIdx);
    }
    else if(gpBle_VsdNullSinkMode == gpHci_VsdSinkMode_SinkRxIndication)
    {
        // Notify the host if needed
        gpHci_EventCbPayload_t params;

        gpHal_GetTime(&params.vsdSinkRxIndication.rxTs);
        params.vsdSinkRxIndication.connHandle = Ble_RxDataBuffers[bufferIdx].connHandle;

        params.vsdSinkRxIndication.dataLength = Ble_RxDataBuffers[bufferIdx].length;

        gpBle_ScheduleEvent(0, gpHci_EventCode_VsdSinkRxIndication, &params);

        gpBle_HciDataRxConfirm(bufferIdx);
    }
    else
#endif /* GP_DIVERSITY_DEVELOPMENT */
    {
        gpBle_cbDataRxIndication(  Ble_RxDataBuffers[bufferIdx].connHandle,
                                   Ble_RxDataBuffers[bufferIdx].flag,
                                   bufferIdx,
                                   Ble_RxDataBuffers[bufferIdx].length,
                                   &Ble_RxDataBuffers[bufferIdx].data[0]);
    }
}

static UInt16 Ble_GetFreeSpace(gpBle_RxBufferHandle_t bufferIdx)
{
    UInt16 maxLength = BLE_DATA_RX_MAX_BUFFER_SIZE;
    UInt16 length = Ble_RxDataBuffers[bufferIdx].length;
    return maxLength - length;
}

static void Ble_AppendToRxBuffer(gpBle_RxBufferHandle_t bufferIdx, gpPd_Loh_t pdLoh)
{
    UInt16 freeSpace = Ble_GetFreeSpace(bufferIdx);
    UInt16 offset = Ble_RxDataBuffers[bufferIdx].length;
    GP_ASSERT_DEV_INT(pdLoh.length <= freeSpace);
    UInt16 length = pdLoh.length; // set to 0 by gpPd_ReadWithUpdate
    gpPd_ReadWithUpdate(&pdLoh, length, &Ble_RxDataBuffers[bufferIdx].data[offset]);
    Ble_RxDataBuffers[bufferIdx].length += length;
}

static Bool Ble_AttemptToResumeFlow(void)
{
    // if flow-stop is currently active AND
    // at least 1 buffer (with WSF memory) is available, then resume the flow
    if ( flow_stop_active &&
         (GetEmptyRxDataBufferIdx() < GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX) )
    {
        // resume the flow
        gpBle_DataRxQueueSetFlowCtrl((UInt16)0x0000);
        flow_stop_active = false;
        return false; // no need to schedule further resume attempts
    }

    return flow_stop_active;
}

#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
static void Ble_GetBuffersAndAttemptToResumeFlow(void)
{
    UInt8 idx;

    // Try to obtain a WSF buffer for all Rx data buffers that don't have one
    for (idx=0;idx<GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX;++idx)
    {
        if ( (GP_HCI_CONNECTION_HANDLE_INVALID == Ble_RxDataBuffers[idx].connHandle) &&
             (NULL == Ble_RxDataBuffers[idx].data) )
        {
            Ble_RxDataBuffers[idx].data = (UInt8*)WsfMsgAlloc(BLE_DATA_RX_MAX_BUFFER_SIZE);
        }
    }

    gpSched_UnscheduleEvent(Ble_GetBuffersAndAttemptToResumeFlow);
    if (Ble_AttemptToResumeFlow())
    {
        gpSched_ScheduleEvent(3000, Ble_GetBuffersAndAttemptToResumeFlow);
    }
}
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpBle_DataRxReset(Bool firstReset)
{
    UInt8 idx;
    GP_LOG_PRINTF("Data RX Reset",0);

    if (firstReset)
    {
        MEMSET(Ble_RxDataBuffers, 0, sizeof(Ble_RxDataBuffers));
    }

    for (idx=0;idx<GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX;++idx)
    {
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
        // all WSF buffers in Ble_RxDataBuffers[].data are owned by the Data Rx module
        // These have not been sent yet to the application - release before re-allocating
        // Except during firstReset (then pointers have not been initialized yet)
        if (NULL != Ble_RxDataBuffers[idx].data)
        {
            WsfMsgFree((void*)Ble_RxDataBuffers[idx].data);
            Ble_RxDataBuffers[idx].data = NULL;
        }
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
        FreeRxDataBufferIdx(idx);
    }

#ifdef GP_DIVERSITY_DEVELOPMENT
    gpBle_VsdFlushAtEndOfConnectionEvent = true;
    gpBle_VsdNullSinkMode = gpHci_VsdSinkMode_Off;
#endif /* GP_DIVERSITY_DEVELOPMENT */

#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    // The OPTIMIZE_MEMCPY flag is set - so we know we're working with a tightly integrated Host stack (Cordio)
    // which triggers gpBle_DataRxReset() twice in the initialization flow:
    // - a first time before the Host stack was configured,
    // - a 2nd time (HCI Reset) after the Host stack (incl WSF memory subsystem) has been initializaed
    if (!firstReset)
    {
        Ble_GetBuffersAndAttemptToResumeFlow();
    }
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
}

void gpBle_DataRxOpenConnection(Ble_IntConnId_t connId)
{
    GP_LOG_PRINTF("Data Rx: open conn %x",0,connId);
    DataRxLinkContext[connId].DataReceived = false;
    DataRxLinkContext[connId].cbUnexpectedDataRx = NULL;
}

void gpBle_DataRxInterceptUnexpectedPdus(Ble_IntConnId_t connId, Ble_cbUnexpectedDataRx_t func)
{
    DataRxLinkContext[connId].cbUnexpectedDataRx = func;
    GP_LOG_PRINTF("Set data rx intercept cb: %lx",0,(unsigned long)func);
}

void gpBle_DataRxCloseConnection(Ble_IntConnId_t connId)
{
    UInt8 bufferIdx;
    gpHci_ConnectionHandle_t connHandle = 0;

    /* Validate the input */
    connHandle = gpBleLlcp_IntHandleToHciHandle(connId);
    GP_ASSERT_DEV_INT(connHandle < GP_HCI_CONNECTION_HANDLE_INVALID);

    /* Only one buffer can be open at the same time for a single connection */
    bufferIdx = GetRxDataBufferIdx(connHandle);
    if (bufferIdx < GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX)
    {
        Ble_FlushRxBuffer(bufferIdx);
    }
}

void gpBle_HciDataRxConfirm(gpBle_RxBufferHandle_t buffHandle)
{
    GP_ASSERT_DEV_INT(buffHandle < GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX);
    GP_ASSERT_DEV_INT(!Ble_RxDataBuffers[buffHandle].isOpen);

    FreeRxDataBufferIdx(buffHandle);

#ifndef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    if (Ble_AttemptToResumeFlow())
    {
        // Weird case: cannot get a new Data Rx buffer - while we just released one
        GP_ASSERT_DEV_EXT(false);
    }
#else
    Ble_GetBuffersAndAttemptToResumeFlow();
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
}

/*
 * Buffers serve as a tool for basic flow control.
 */
void gpBle_DataRxIndication(Ble_IntConnId_t connId, Ble_LLID_t llid, gpPd_Loh_t pdLoh)
{
    gpHci_ConnectionHandle_t connHandle = 0;
    gpHci_PacketBoundaryFlag_t flag = gpHci_L2CAP_Invalid;

    GP_LOG_PRINTF("Data Rx Ind",0);

    /* Validate the input */
    if(!gpBle_IsConnectionOpen(connId))
    {
        // Connections presumably closed before all interrupt were handled (e.g. reset).
        // TODO: to never have this case would require flushing all pending events
        // in RT subsystem prior to destroying the context in the NRT systen.
        GP_LOG_PRINTF("Warning: gpBle_DataRxIndication: connId=%i unknown!",0,connId);
        Ble_RMFreeResource(connId, pdLoh.handle);
        return;
    }

    connHandle = gpBleLlcp_IntHandleToHciHandle(connId);
    GP_ASSERT_DEV_INT(connHandle < GP_HCI_CONNECTION_HANDLE_INVALID); // per construction, an active connnection has a valid Hci Handle

    if(DataRxLinkContext[connId].cbUnexpectedDataRx)
    {
        DataRxLinkContext[connId].cbUnexpectedDataRx(connId);
        return;
    }

    if(GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_RX < pdLoh.length)
    {
        // The peer (knowingly) sent a PDU with length > what we can handle
        // The peer should know our max Rx PDU size (cfr DLE procedure)
        Ble_RMFreeResource(connId, pdLoh.handle);

        // Do not worry w.r.t. inhibiting slave latency
        // The peer is misbehaving - might be spamming us
        return;
    }

#ifdef GP_DIVERSITY_DEVELOPMENT
    if(gpBle_VsdNullSinkMode != gpHci_VsdSinkMode_Off)
    {
        if(gpBle_VsdNullSinkMode == gpHci_VsdSinkMode_SinkRxIndication)
        {
            GP_LOG_PRINTF("!VsdSinkRxIndication",0);
            // Notify the host if needed
            gpHci_EventCbPayload_t params;

            params.vsdSinkRxIndication.connHandle = connHandle;
            params.vsdSinkRxIndication.dataLength = pdLoh.length;
            params.vsdSinkRxIndication.rxTs = gpPd_GetRxTimestamp(pdLoh.handle);

            gpBle_ScheduleEvent(0, gpHci_EventCode_VsdSinkRxIndication, &params);
        }

        Ble_RMFreeResource(connId, pdLoh.handle);

        return;
    }
#endif /* GP_DIVERSITY_DEVELOPMENT */

    if(llid == Ble_LLID_DataStart)
    {
        flag = gpHci_L2CAP_Start_AutoFlush;
    }
    else if(llid == Ble_LLID_DataContinuedOrEmpty)
    {
        flag = gpHci_L2CAP_Continue;
    }
    else
    {
        GP_ASSERT_DEV_INT(false);
    }

    /* Find/Allocate a data buffer */
    gpBle_RxBufferHandle_t bufferIdx = GetRxDataBufferIdx(connHandle);
    if (bufferIdx == GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX)
    {
        bufferIdx = AllocRxDataBufferIdx(connHandle, flag);
    }
    else if (Ble_GetFreeSpace(bufferIdx) < pdLoh.length)
    {
        GP_LOG_PRINTF("DRx flsh: free < inc len",0);
        Ble_FlushRxBuffer(bufferIdx);
        bufferIdx = AllocRxDataBufferIdx(connHandle, flag);
    }
    else if (flag == gpHci_L2CAP_Start_AutoFlush)
    {
        GP_LOG_PRINTF("DRx flsh: Rx nw strt frag",0);
        Ble_FlushRxBuffer(bufferIdx);
        bufferIdx = AllocRxDataBufferIdx(connHandle, flag);
    }
    // Sanity check: if no valid buffer, the Rx flow should have been stopped before
    GP_ASSERT_DEV_EXT(bufferIdx < GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX);
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    GP_ASSERT_DEV_EXT(NULL != Ble_RxDataBuffers[bufferIdx].data);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
    Ble_AppendToRxBuffer(bufferIdx, pdLoh);

    /* No more buffers available, stop accepting */
    if (GetEmptyRxDataBufferIdx() == GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX)
    {
        GP_LOG_PRINTF("DRx flsh: all buff full",0);
        /* TODO: [Optimize] while freeSpace >= max PDU size connection
         * of the current packet can keep flowing */
        gpBle_DataRxQueueSetFlowCtrl((UInt16)0xFFFF);
        flow_stop_active = true;
        Ble_FlushRxBuffer(bufferIdx);
    }
    else if (Ble_GetFreeSpace(bufferIdx) < gpBle_GetEffectiveMaxRxOctets(connId))
    {
        GP_LOG_PRINTF("DataRx flush: free=%d < max PDU=%d",0,Ble_GetFreeSpace(bufferIdx),gpBle_GetEffectiveMaxRxOctets(connId));
        Ble_FlushRxBuffer(bufferIdx);
    }

    Ble_RMFreeResource(connId, pdLoh.handle);

    if (!DataRxLinkContext[connId].DataReceived)
    {
        gpBleLlcp_ProhibitSlaveLatency(connId, true, Ble_ProhibitSlaveLatency_DataRx);
        DataRxLinkContext[connId].DataReceived = true;
    }
}

void gpBle_DataRxConnEventDone(Ble_IntConnId_t connId)
{
#ifdef GP_DIVERSITY_DEVELOPMENT
    if (gpBle_VsdFlushAtEndOfConnectionEvent)
#endif /* GP_DIVERSITY_DEVELOPMENT */
    {
        // coincidentaly, take same actions as for closing a connection between 2 devices
        // FixMe : better name for Ble_DataRxCloseConnection
        gpBle_DataRxCloseConnection(connId);
    }

    if (!DataRxLinkContext[connId].DataReceived)
    {
        gpBleLlcp_ProhibitSlaveLatency(connId, false, Ble_ProhibitSlaveLatency_DataRx);
    }
    else
    {
        DataRxLinkContext[connId].DataReceived = false;
    }
}

#ifdef GP_DIVERSITY_DEVELOPMENT
void Ble_SetVsdDataRxFlushAtEndOfEvent(Bool enable)
{
    gpBle_VsdFlushAtEndOfConnectionEvent = enable;
}

gpHci_Result_t gpBle_VsdSetNullSinkEnable(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    if(pParams->VsdSetNullSinkEnable.mode >= gpHci_VsdSinkMode_Invalid)
    {
        return gpHci_ResultInvalidHCICommandParameters;
    }

    gpBle_VsdNullSinkMode = pParams->VsdSetNullSinkEnable.mode;

    return gpHci_ResultSuccess;
}

#endif /* GP_DIVERSITY_DEVELOPMENT */
