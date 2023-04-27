/*
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

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLELLCPPROCEDURES

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleConfig.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcpProcedures_defs.h"
#include "gpBleLlcpProcedures_Update_defs.h"
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpSched.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_CONN_EVENT_INSTANT_OFFSET       1

#define BLE_CONN_EVENT_COUNT_MAX            0xFFFF
#define BLE_CONN_EVENT_COUNT_HALF           (BLE_CONN_EVENT_COUNT_MAX >> 1)
#define BLE_CONN_EVENT_COUNT_OVERFLOW       (BLE_CONN_EVENT_COUNT_MAX + 1)

#define BLE_MIN_NR_OF_CONN_EVENTS_BEFORE_TO         6

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

// Instant is one event after last valid, apply correction when instant is zero
#define BLE_INSTANT_TO_LAST_CONN_EVENT(instant) ((instant == 0) ? 0xFFFF: (instant - 1))

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
*****************************************************************************/

static Ble_LlcpProcedureContext_t* Ble_LlcpGetActiveInstantProcedure(Ble_LlcpLinkContext_t* pContext);
static void Ble_LlcpPriorityIncreaseFollowUp(void* pArg);
static gpHal_Result_t Ble_LlcpImmediatePauseConnEvent(Ble_LlcpProcedureContext_t* pProcedure);
// Callback from gpHal indicating that the last scheduled event counter has passed
static void Ble_LlcpLastSchedEventPassed(Ble_IntConnId_t connId);
// Called when the event before the instant is reached (from Ble_LlcpLastSchedEventPassed)
static void Ble_LlcpPreInstantPassed(Ble_IntConnId_t connId, Bool scheduleInstantPassed);
// Called when the instant has passed (one interval after Ble_LlcpPreInstantPassed)
static void Ble_LlcpInstantPassed(void* pArg);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Ble_LlcpProcedureContext_t* Ble_LlcpGetActiveInstantProcedure(Ble_LlcpLinkContext_t* pContext)
{
    UIntLoop i;
    UInt8 procCounter = 0;
    Ble_LlcpProcedureContext_t* pProcedureTmp = NULL;
    Ble_LlcpProcedureContext_t* pProcedure = NULL;

    for(i = 0; i < 2; i++)
    {
        pProcedureTmp = gpBleLlcpFramework_GetProcedure(pContext, i);

        if(pProcedureTmp && gpBleLlcpFramework_ProcedureStateGet(pProcedureTmp, BLE_LLCP_PROCEDURE_WAITING_ON_INSTANT_IDX))
        {
            pProcedure = pProcedureTmp;
            procCounter++;
        }
    }

    // It is an error when 2 instant procedures are running simultaneously
    GP_ASSERT_DEV_INT(procCounter != 2);

    return pProcedure;
}

void Ble_LlcpLastSchedEventPassed(Ble_IntConnId_t connId)
{
    Ble_LlcpPreInstantPassed(connId, true);
}

void Ble_LlcpPreInstantPassed(Ble_IntConnId_t connId, Bool scheduleInstantPassed)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;
    UInt32 currentIntervalUs;

    pContext = Ble_GetLinkContext(connId);

    if(pContext == NULL)
    {
        GP_ASSERT_DEV_INT(false);
        return;
    }
    GP_LOG_PRINTF("Connection %u: pre-instant passed",0, pContext->connId);

    pProcedure = Ble_LlcpGetActiveInstantProcedure(pContext);

    if(pProcedure == NULL)
    {
        // pProcedure is allowed to be NULL only when termination is ongoing
        GP_ASSERT_DEV_INT(pContext->terminationOngoing);
        return;
    }

    currentIntervalUs = BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit);

    gpHal_SetConnectionPriority(pContext->connId, Ble_Priority_VeryHigh);
    gpSched_UnscheduleEventArg(Ble_LlcpPriorityIncreaseFollowUp, pContext);
    gpSched_ScheduleEventArg(currentIntervalUs*BLE_MIN_NR_OF_CONN_EVENTS_BEFORE_TO,
                             Ble_LlcpPriorityIncreaseFollowUp, pContext);

    switch(pProcedure->procedureId)
    {
        case gpBleLlcp_ProcedureIdConnectionUpdate:
        case gpBleLlcp_ProcedureIdConnectionParamRequest:
        {
            Ble_LlcpConnectionUpdatePreInstantPassed(pContext, pProcedure);
            break;
        }
        case gpBleLlcp_ProcedureIdChannelMapUpdate:
        {
            Ble_LlcpChannelMapUpdatePreInstantPassed(pContext, pProcedure);
            break;
        }
#ifdef GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
        case gpBleLlcp_ProcedureIdPhyUpdate:
        {
            Ble_LlcpPhyUpdatePreInstantPassed(pContext, pProcedure);
            break;
        }
#endif //GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
        default:
        {
            // Invalid procedure
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    if(scheduleInstantPassed)
    {
        gpSched_ScheduleEventArg(currentIntervalUs, Ble_LlcpInstantPassed, (void*)pContext);
    }
}

/* called after every connection event done */
void gpBleLlcpProcedures_ConnectionEventDone(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;
    pContext = Ble_GetLinkContext(connId);

    if(pContext == NULL)
    {
        GP_ASSERT_DEV_INT(false);
        return;
    }


    /* In the context of SDP004-2186
     * Why is "Ble_LlcpInstantPassed" sheduled in the first place?
     * We should trigger it from "ConnectionEventDone". */
    /* if the "Ble_LlcpInstantPassed" function is scheduled this means
     * the connection update procedure is not finished yet in the NRT
     * while the RT is already processing connection events with the new
     * connection paramters. This can happen when the transmitWindowOffset is
     * smaller than the old connection interval.
     * While the procedure is not finished the ActivityManager will still see
     * the old connection parameters and when, for example, the next anchor time
     * is far into the future while the old supervision time is smaller then
     * it can falsly conclude the supervision time has been reached.
     */
    if (gpSched_ExistsEventArg(Ble_LlcpInstantPassed, (void*)pContext))
    {
        gpSched_UnscheduleEventArg(Ble_LlcpInstantPassed, (void*)pContext);
        // Immediately finish the procedure.
        Ble_LlcpInstantPassed(pContext);
    }
}

void Ble_LlcpInstantPassed(void* pArg)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;

    pContext = (Ble_LlcpLinkContext_t*)pArg;

    if(pContext == NULL)
    {
        GP_ASSERT_DEV_INT(false);
        return;
    }

    GP_LOG_PRINTF("Connection %u: instant passed",0, pContext->connId);

    pProcedure = Ble_LlcpGetActiveInstantProcedure(pContext);

    if(pProcedure == NULL)
    {
        // pProcedure is allowed to be NULL only when termination is ongoing
        GP_ASSERT_DEV_INT(pContext->terminationOngoing);
        return;
    }

    // When the instant has passed, we need to stop the active instant procedure
    gpBleLlcpFramework_StopActiveProcedure(pContext->hciHandle, pProcedure->localInit, gpHci_ResultSuccess);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_UpdateCommonInit(gpHal_BleCallbacks_t* pCallbacks)
{
    // We are interested in last scheduled event counter callbacks
    pCallbacks->cbLastSchedEventPassed = Ble_LlcpLastSchedEventPassed;
}

void Ble_LlcpProcedureUpdateResetConnection(Ble_LlcpLinkContext_t* pContext)
{
    gpSched_UnscheduleEventArg(Ble_LlcpPriorityIncreaseFollowUp, pContext);
    while(gpSched_UnscheduleEventArg(Ble_LlcpInstantPassed, pContext));
}

UInt16 Ble_LlcpCalculateProcedureInstant(Ble_LlcpLinkContext_t* pContext, UInt16 currentEventCount)
{
    // Can overflow, but connection event counter uses full 16 bits, so this will not give any problems
    // see BT SIG Erratum 7034 for handling slave latency
    UInt16 instant = currentEventCount + ((pContext->latency+1)*6) + BLE_CONN_EVENT_INSTANT_OFFSET;
    return instant;
}

gpHal_Result_t Ble_LlcpImmediatePauseConnEvent(Ble_LlcpProcedureContext_t* pProcedure)
{
    gpHal_Result_t result = gpHal_ResultSuccess;
    Ble_LlcpLinkContext_t* pContext;

    GP_LOG_SYSTEM_PRINTF("ImmediatePauseConnEvent",0);

    pContext = Ble_GetLinkContext(pProcedure->connId);

    if(pContext == NULL)
    {
        GP_ASSERT_DEV_INT(pContext);
        return gpHal_ResultInvalidParameter;
    }

    result = gpHal_BlePauseConnectionEvent(pProcedure->connId);

    if(result != gpHal_ResultSuccess)
    {
        GP_ASSERT_DEV_INT(false);
        return result;
    }

    gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_INSTANT_IDX);

    Ble_LlcpPreInstantPassed(pProcedure->connId, false);
    Ble_LlcpInstantPassed(pContext);

    return result;
}

void Ble_LlcpConfigureLastScheduledConnEvent(Ble_LlcpProcedureContext_t* pProcedure, UInt16 instant)
{
    gpHal_Result_t result;
    Ble_ProhibitSlaveLatency_Mask_t mask;
    mask = pProcedure->localInit ? Ble_ProhibitSlaveLatency_LocalProcedure : Ble_ProhibitSlaveLatency_RemoteProcedure;

    /* Re-enable slave latency while waiting for the instant so we don't prohibit sleep (APP-6571) */
    gpBleLlcp_ProhibitSlaveLatency(pProcedure->connId, false, mask);

    result = gpHal_BleSetLastScheduledConnEventCount(pProcedure->connId, BLE_INSTANT_TO_LAST_CONN_EVENT(instant));

    gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_INSTANT_IDX);

    // should always be success when called from here
    GP_ASSERT_DEV_INT(result == gpHci_ResultSuccess);
}

/* Note that this must hold true: currentEventCount <= instant !!! */
void Ble_LlcpConfigureLastScheduledConnEventAfterCurrent(Ble_LlcpProcedureContext_t* pProcedure, UInt16 currentEventCount, UInt16 instant)
{
    gpHal_Result_t result;
    UInt16 next_currentEventCount;

    /* Try to configure the last*/
    Ble_LlcpConfigureLastScheduledConnEvent(pProcedure, instant);

    /* We check again that we have not yet reached the instant ... */
    next_currentEventCount = gpHal_BleGetCurrentConnEventCount(pProcedure->connId);

    /* If we are already at the instant we should stop the event right away! */
    if (BLE_EC_CMP(currentEventCount, next_currentEventCount, instant) >= 0)
    {
        result = gpHal_BleStopLastScheduledConnEventCount(pProcedure->connId);
        GP_ASSERT_DEV_INT(result == gpHci_ResultSuccess);
        Ble_LlcpImmediatePauseConnEvent(pProcedure);
    }
}

/* Note that this must hold true: passedEventCount <= currentConnEventCount !!!
 *   e.g. passedEventCount == event count of instant-conveying PDU */
void Ble_LlcpConfigureLastScheduledConnEventAfterPassed(Ble_LlcpProcedureContext_t* pProcedure, UInt16 passedEventCount, UInt16 instant)
{
    UInt16 currentConnEventCount;

    currentConnEventCount = gpHal_BleGetCurrentConnEventCount(pProcedure->connId);

    /* Only if the instant is in the future we set the last scheduled event count ... */
    if (BLE_EC_CMP(passedEventCount, currentConnEventCount, instant) < 0)
    {
        Ble_LlcpConfigureLastScheduledConnEventAfterCurrent(pProcedure, currentConnEventCount, instant);
    }
    else
    {
        Ble_LlcpImmediatePauseConnEvent(pProcedure);
    }
}

UInt16 Ble_LlcpGetPduConnEventCount(Ble_LlcpLinkContext_t* pContext, gpPd_Loh_t* pPdLoh)
{
    gpHal_ConnEventMetrics_t connMetrics;
    UInt16 pduConnEventCount;
    gpPd_TimeStamp_t ts;
    UInt32 timeDiff;

    ts = gpPd_GetRxTimestamp(pPdLoh->handle);
    //gpPd_TimeStamp_t tsChip = gpPd_GetRxTimestampChip(pPdLoh->handle); ???

    // calculate the eventCount that corresponds to the RX timestamp
    gpHal_BleGetConnectionMetrics(pContext->connId, &connMetrics);


    /* RT might not yet have updated the nextAnchorTime by the time the PDU is being processed
     * so we can only be sure the RX timestamp will be <(nextAnchorTime+interval) ! */
    timeDiff = BLE_GET_TIME_DIFF(ts, connMetrics.nextAnchorTime + BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit));
    pduConnEventCount = connMetrics.eventCounterNext - (timeDiff / BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit));

    return pduConnEventCount;
}

Bool Ble_LlcpInstantValid(UInt16 instant, UInt16 currentConnEventCount)
{
    Int32 connEventCountDiff;

    connEventCountDiff = instant - currentConnEventCount;

    if(connEventCountDiff <= 0)
    {
        connEventCountDiff += BLE_CONN_EVENT_COUNT_OVERFLOW;
    }

    if(BLE_CONN_EVENT_COUNT_OVERFLOW - connEventCountDiff < BLE_CONN_EVENT_COUNT_HALF)
    {
        return false;
    }

    return true;
}

// Gets called when an update PDU is received with an invalid instant (in the past)
void Ble_LlcpInstantInvalidFollowUp(void* pArg)
{
    Ble_LlcpLinkContext_t* pContext = (Ble_LlcpLinkContext_t*)pArg;

    GP_LOG_SYSTEM_PRINTF("instant invalid => stop",0);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(pContext->connId));

    gpBle_StopConnection(pContext->hciHandle, gpHci_ResultInstantPassed);
}

void Ble_LlcpPriorityIncreaseFollowUp(void* pArg)
{
    Ble_LlcpLinkContext_t* pContext = (Ble_LlcpLinkContext_t*)pArg;

    GP_LOG_PRINTF("Set priority of conn %x back down to normal",0,pContext->connId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(pContext->connId));

    gpHal_SetConnectionPriority(pContext->connId, BLE_CONN_PRIORITY);
}

void Ble_LlcpStopLastScheduledConnEventCount(Ble_LlcpLinkContext_t* pContext)
{
    while(gpSched_UnscheduleEventArg(Ble_LlcpPriorityIncreaseFollowUp, pContext));
    gpHal_BleStopLastScheduledConnEventCount(pContext->connId);
}

