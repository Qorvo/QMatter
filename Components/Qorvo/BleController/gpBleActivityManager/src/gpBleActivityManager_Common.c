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

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEACTIVITYMANAGER

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpBleActivityManager.h"
#include "gpBleActivityManager_defs.h"
#include "gpSched.h"
#include "gpBleConfig.h"

#if defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleAdvertiser.h"
#endif //GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_PERIPHERAL

#if defined(GP_DIVERSITY_BLE_OBSERVER) 
#include "gpBleScanner.h"
#endif //GP_DIVERSITY_BLE_OBSERVER || GP_DIVERSITY_BLE_CENTRAL

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleInitiator.h"
#include "gpBleLlcp.h"
#include "gpBle_LLCP_getters.h"
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

// The duration of the supervision timeout during connection creation
#define BLE_MONITOR_CONN_CREATION_TIMEOUT_INTERVALS        6U

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

static void BleActivityManager_ResetSupervisionTimeout(gpHci_ConnectionHandle_t connHandle, Bool reschedule, UInt32 reschedToUs);
static void BleActivityManager_ConnectionLost(BleActivityManager_EventProcessed_t* pContext, gpHal_ConnEventMetrics_t* pMetrics, gpHci_Result_t reason);
static void BleActivityManager_ConnectionSupervisionTimeout(void* pArgs);


static void BleActivityManager_NotifyHost(BleActivityManager_EventProcessed_t* pContext, gpHal_ConnEventMetrics_t* pMetrics);
static void BleActivityManager_EstablishConnectionIfPossible(BleActivityManager_EventProcessed_t* pContext, Ble_MonitorLinkContext_t* pMonitorContext, gpHal_ConnEventMetrics_t* pMetrics);
static void BleActivityManager_ProcessEstablished(BleActivityManager_EventProcessed_t* pContext, Ble_MonitorLinkContext_t* pMonitorContext, gpHal_ConnEventMetrics_t* pMetrics);
static void BleActivityManager_ProcessNotEstablished(BleActivityManager_EventProcessed_t* pContext, gpHal_ConnEventMetrics_t* pMetrics);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void BleActivityManager_ResetSupervisionTimeout(gpHci_ConnectionHandle_t connHandle, Bool reschedule, UInt32 reschedToUs)
{
    void* argument = (void*)((UIntPtr)connHandle + 1);

    gpSched_UnscheduleEventArg(BleActivityManager_ConnectionSupervisionTimeout, argument);

    if(reschedule)
    {
        gpSched_ScheduleEventArg(reschedToUs, BleActivityManager_ConnectionSupervisionTimeout, argument);
    }
}

void BleActivityManager_ConnectionLost(BleActivityManager_EventProcessed_t* pContext, gpHal_ConnEventMetrics_t* pMetrics, gpHci_Result_t reason)
{

    gpBle_StopConnection(pContext->hciHandle, reason);

}

void BleActivityManager_ConnectionSupervisionTimeout(void* pArgs)
{
    gpHci_ConnectionHandle_t connHandle = (gpHci_ConnectionHandle_t)((UIntPtr)pArgs - 1);
    BleActivityManager_EventProcessed_t eventProcessed;

    gpBleActivityManager_GetEventProcessedContext(connHandle, &eventProcessed);

    gpHal_ConnEventMetrics_t metrics;
    Ble_MonitorLinkContext_t monitorContext;
    gpHci_Result_t terminationReason;

    if(!BleActivityManager_GetMonitorContext(eventProcessed.connId, &monitorContext) ||
      (eventProcessed.connId == BLE_CONN_HANDLE_INVALID) || eventProcessed.terminationOngoing)
    {
        //Connection is about to close/ closed. Nothing to do.
        return;
    }

    GP_LOG_SYSTEM_PRINTF("[WARN] HCI connection %u: NRT Link supervision TO!",0, eventProcessed.hciHandle);
    gpHal_BleGetConnectionMetrics(eventProcessed.connId, &metrics);

    if(monitorContext.connectionEstablished)
    {
        terminationReason = gpHci_ResultConnectionTimeout;
    }
    else
    {
        terminationReason = gpHci_ResultConnectionFailedtobeEstablished;
    }

    BleActivityManager_ConnectionLost(&eventProcessed, &metrics, terminationReason);
}


void BleActivityManager_NotifyHost(BleActivityManager_EventProcessed_t* pContext, gpHal_ConnEventMetrics_t* pMetrics)
{

}

void BleActivityManager_EstablishConnectionIfPossible(BleActivityManager_EventProcessed_t* pContext, Ble_MonitorLinkContext_t* pMonitorContext, gpHal_ConnEventMetrics_t* pMetrics)
{
    if(!pMonitorContext->connectionEstablished)
    {
        // tsLastPacketReceived must be initialized to initialCorrelationTs (which is initialized to the timestamp of the first real connection event being scheduled)
        if (BLE_GET_TIME_DIFF(pContext->initialCorrelationTs, pMetrics->tsLastPacketReceived) > 0)
        {
            // So we have received a data channel PDU with correct or incorrect CRC: the connection is now established
            pMonitorContext->connectionEstablished = true;

            gpBle_EstablishConnection(pContext->hciHandle, gpHci_ResultSuccess);
            // The initial supervision timeout can be unscheduled
            BleActivityManager_ResetSupervisionTimeout(pContext->hciHandle, false, 0);
        }
    }
}

void BleActivityManager_ProcessEstablished(BleActivityManager_EventProcessed_t* pContext, Ble_MonitorLinkContext_t* pMonitorContext, gpHal_ConnEventMetrics_t* pMetrics)
{
    // Reschedule the NRT link supervision timer
    // During a connection update, the NRT supervision timer is stopped.
    // tsLastValidPacketReceived is also updated starting from the instant.
    // This means that the NRT timer will always be restarted after the instant.

    static UInt32 lastTs = 0;
    UInt32 timeDiff = BLE_GET_TIME_DIFF(pMetrics->tsLastValidPacketReceived, pMetrics->nextAnchorTime);

    if(lastTs != pMetrics->tsLastValidPacketReceived)
    {
        UInt32 offset;
        UInt32 timeout;

        lastTs = pMetrics->tsLastValidPacketReceived;
        offset = BLE_GET_TIME_DIFF(lastTs, gpSched_GetCurrentTime());
        timeout = BLE_TIME_UNIT_10000_TO_US(pContext->timeoutUnit);

        if(timeout < offset)
        {
            GP_LOG_SYSTEM_PRINTF("[WARN] HCI connection %u: NRT Link supervision TO!",0, pContext->hciHandle);
            BleActivityManager_ConnectionLost(pContext, pMetrics, gpHci_ResultConnectionTimeout);
            return;
        }

        BleActivityManager_ResetSupervisionTimeout(pContext->hciHandle, true, (timeout - offset));
    }

    // we need to check vs LSTO + 1 extra interval, since we calculate the Link Supervision Timer (timeDiff) based on nextAnchorTime
    if (timeDiff >= (BLE_TIME_UNIT_10000_TO_US(pContext->timeoutUnit) + BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit)))
    {
        GP_LOG_SYSTEM_PRINTF("[WARN] HCI connection %u: RT Link supervision TO!",0, pContext->hciHandle);
        BleActivityManager_ConnectionLost(pContext, pMetrics, gpHci_ResultConnectionTimeout);
        return;
    }

    // window widening only applies for slave connections
    if (!pContext->centralConnection)
    {
        if (pMetrics->windowDuration >= BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit)) // windowDuration = 2 * windowWidening : cfr BT v4.2 Vol6 Part B $4.5.7  note that we can safely ignore T_IFS in this calculation
        {
            pMonitorContext->slaveNbrConsecutiveLargeCorrWindows += 1;

            // The windowDuration shows the total correlator window size for the next connection event (cfr SW arch doc $5.5.1)
            // Only drop the link after we observed 2 consecutive large windowDuration >= Interval
            if (pMonitorContext->slaveNbrConsecutiveLargeCorrWindows >= 2)
            {
                GP_LOG_SYSTEM_PRINTF("[WARN] HCI connection %u: RX window too large!",0, pContext->hciHandle);
                BleActivityManager_ConnectionLost(pContext, pMetrics, gpHci_ResultConnectionTimeout);
                return;
            }
        }
        else
        {
            pMonitorContext->slaveNbrConsecutiveLargeCorrWindows = 0;
        }
    }
}

void BleActivityManager_ProcessNotEstablished(BleActivityManager_EventProcessed_t* pContext, gpHal_ConnEventMetrics_t* pMetrics)
{
    // From the spec: If the Link Layer connection supervision timer reaches 6 * connInterval before
    // the connection is established (see Section 4.5), the connection shall be considered lost.
    // The timer starts at the first anchor point. This means that the timer ends exactly at the 7th anchor point.
    // Since this timeout is only checked at the end of the connection event, we cannot fire it at the exact moment.
    // For a master, we know in advance if the timeout will reach 6 * connInterval (as we determine the timing).
    // As a slave, we don't know that, so we need to listen an additional event and determine afterwards whether or not
    // the timeout was reached.
    // See also LlConAdvBv02C (tests timing of disconnect event).

    UInt32 timeDiff = BLE_GET_TIME_DIFF(pMetrics->tsLastValidPacketReceived, pMetrics->nextAnchorTime);
    UInt8 nrOfEventsBeforeTimeout = BLE_MONITOR_CONN_CREATION_TIMEOUT_INTERVALS;

    if(!pContext->centralConnection)
    {
        nrOfEventsBeforeTimeout++;
    }

    if (timeDiff >= (nrOfEventsBeforeTimeout) * BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit))
    {
        GP_LOG_SYSTEM_PRINTF("[WARN] HCI connection %u: Could not be established!",0, pContext->hciHandle);
        gpBle_EstablishConnection(pContext->hciHandle, gpHci_ResultConnectionFailedtobeEstablished);
        BleActivityManager_ConnectionLost(pContext, pMetrics, gpHci_ResultConnectionFailedtobeEstablished);
        return;
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleActivityManager_InitCommon(void)
{

}

void gpBleActivityManager_ResetCommon(void)
{
    for(UInt8 i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        Ble_LlcpLinkContext_t* pContext = Ble_GetLinkContext(i);

        if(pContext)
        {
            BleActivityManager_ResetSupervisionTimeout(pContext->hciHandle, false, 0);
        }
    }
}

void gpBleActivityManager_OpenConnection(Ble_IntConnId_t connId, UInt32 firstConnEventTs)
{
    UInt32 tNow;
    UInt32 supervisionToUs;

    HAL_TIMER_GET_CURRENT_TIME_1US(tNow);

    if(gpBle_IsTimestampEarlier(tNow, firstConnEventTs))
    {
        supervisionToUs = BLE_GET_TIME_DIFF(tNow, firstConnEventTs);
    }
    else
    {
        supervisionToUs = BLE_GET_TIME_DIFF(firstConnEventTs, tNow);
    }

    supervisionToUs += (BLE_MONITOR_CONN_CREATION_TIMEOUT_INTERVALS * gpBleLlcp_GetConnIntervalUs(connId));

    BleActivityManager_ResetSupervisionTimeout(gpBleLlcp_IntHandleToHciHandle(connId), true, supervisionToUs);
}

void gpBleActivityManager_OnCloseConnectionCommon(Ble_IntConnId_t connId)
{
    //Stop Connection Supervision Timeout
    if(BLE_IS_INT_CONN_HANDLE_VALID(connId))
    {
        BleActivityManager_ResetSupervisionTimeout(gpBleLlcp_IntHandleToHciHandle(connId), false, 0);
    }
}

void gpBleActivityManager_StopSupervisionTimeout(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext = Ble_GetLinkContext(connId);
    if(pContext == NULL)
    {
        return;
    }
    if((pContext->connId == BLE_CONN_HANDLE_INVALID) || pContext->terminationOngoing)
    {
        //Connection is about to close/ closed. Nothing to do.
        return;
    }

    BleActivityManager_ResetSupervisionTimeout(pContext->hciHandle, false, 0);
}

void gpBleActivityManager_ConnectionEventDoneCommon(BleActivityManager_EventProcessed_t* pContext, gpHal_ConnEventMetrics_t* pMetrics)
{
    Ble_MonitorLinkContext_t monitorContext;

    if(!BleActivityManager_GetMonitorContext(pContext->connId, &monitorContext))
    {
        // Drop request when no context can be found
        GP_ASSERT_DEV_INT(false);
        return;
    }

    BleActivityManager_EstablishConnectionIfPossible(pContext, &monitorContext, pMetrics);
    BleActivityManager_NotifyHost(pContext, pMetrics);

    if(monitorContext.connectionEstablished)
    {
        BleActivityManager_ProcessEstablished(pContext, &monitorContext, pMetrics);
    }
    else
    {
        BleActivityManager_ProcessNotEstablished(pContext, pMetrics);
    }

    BleActivityManager_UpdateMonitorContext(&monitorContext);

}

void gpBleActivityManager_GetEventProcessedContext(gpHci_ConnectionHandle_t hciHandle, BleActivityManager_EventProcessed_t* pEvent)
{
    Ble_LlcpLinkContext_t* pContext = Ble_GetLinkContext(gpBleLlcp_HciHandleToIntHandle(hciHandle));

    MEMSET(pEvent, 0, sizeof(BleActivityManager_EventProcessed_t));

    if(pContext == NULL)
    {
        pEvent->connId = Ble_IntConnId_Invalid;
        pEvent->hciHandle = GP_HCI_CONNECTION_HANDLE_INVALID;
        return;
    }

    pEvent->accessAddress = pContext->accessAddress;
    pEvent->connId = pContext->connId;
    pEvent->terminationOngoing = pContext->terminationOngoing;
    pEvent->hciHandle = pContext->hciHandle;
    pEvent->initialCorrelationTs = pContext->initialCorrelationTs;
    pEvent->timeoutUnit = pContext->timeoutUnit;
    pEvent->intervalUnit = pContext->intervalUnit;
    pEvent->centralConnection = pContext->masterConnection;
}

