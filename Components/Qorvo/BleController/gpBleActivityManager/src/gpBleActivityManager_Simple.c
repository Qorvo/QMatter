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
#include "gpRandom.h"
#include "gpBleActivityManager.h"
#include "gpBleActivityManager_defs.h"

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBle_LLCP_getters.h"
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_ACTIVITY_MANAGER_START_TIME_DELAY_US        50000
#define BLE_ACTIVITY_MANAGER_MIN_LISTENING_INTERVALS    6
#define BLE_ACTIVITY_MANAGER_INTERVAL_SAFETY_MARGIN     2
#define BLE_ACTIVITY_MANAGER_ACTIVITY_ID_ADVERTISING    0x80
#define BLE_ACTIVITY_MANAGER_NUMBER_OF_ACTIVITIES   (BLE_LLCP_MAX_NR_OF_CONNECTIONS + BLE_LLCP_MAX_NR_OF_VIRTUAL_CONNECTIONS)

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_CONNID_TO_INDEX(connId) ((connId & GPHAL_BLE_VIRTUAL_CONN_MASK) ? (BLE_LLCP_MAX_NR_OF_CONNECTIONS + (connId & ~GPHAL_BLE_VIRTUAL_CONN_MASK)) : connId)

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// Link context for each connection
static Ble_MonitorLinkContext_t Ble_MonitorLinkContext[BLE_ACTIVITY_MANAGER_NUMBER_OF_ACTIVITIES];


/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void BleActivityManager_GetStartTimeDummy(UInt32* pTimestamp);

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
static UInt32 BleActivityManager_CalculateMasterAnchorPoint(Ble_ActivityId_t connId, UInt16 intervalUnit);
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
void BleActivityManager_GetStartTimeDummy(UInt32* pTimestamp)
{
    gpHal_GetTime(pTimestamp);

    *pTimestamp += BLE_ACTIVITY_MANAGER_START_TIME_DELAY_US;
}

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
UInt32 BleActivityManager_CalculateMasterAnchorPoint(Ble_ActivityId_t connId, UInt16 intervalUnit)
{
    UInt32 intervalT;
    UInt32 unitSize;
    UInt32 betterTs;
    UInt32 firstVirtualTs;
    UInt8 index = BLE_CONNID_TO_INDEX(connId);

    /* We noticed that for different connections the 'firstVirtConnEventTs modulo connection_interval'
     * of these connections are often close to each other (<1000us). Even for large intervals!
     * Root cause unkown. Should be random. To work around this issue we spread them out.
     * Random spreading won't suffice for connections with small intervals anyway!
     */
    BleActivityManager_GetStartTimeDummy(&firstVirtualTs);

    /* We allign the 'firstVirtConnEventTs' onto point number 'connId' in a range
     * of 'BLE_LLCP_MAX_NR_OF_CONNECTIONS' points spread out evenely in a timeframe
     * of size 'connInterval'.
     */
    betterTs = firstVirtualTs;

    intervalT = BLE_TIME_UNIT_1250_TO_US(intervalUnit);
    unitSize = intervalT/BLE_LLCP_MAX_NR_OF_CONNECTIONS;

    betterTs /= intervalT; /* round down */
    betterTs *= intervalT;

    /* We add an extra offset so that this ConnEvent does not overlap half the time with a connection
     * whose interval is twice as large. e.g. Assuming BLE_LLCP_MAX_NR_OF_CONNECTIONS == 9 for
     * (connid=0,interval=12) and (connid=1,interval=6):
     *
     * Since 0*(1250*12/9)==0 and 1*(1250*6/9)==833 and since the connection event for an empty PDU
     * time takes about 910us these connections would compete for the same time on 0,12,24,...
     *
     * Using offsets 12/4==3 and 6/4==1 we see:
     *   conn0:     3       15        27        39        51
     *   conn1:   1   7  13    19  25    31  37    43  49
     *
     * Might not work well for some corner cases? TODO: check the math!
     */
    betterTs += intervalT / 4;

    betterTs += index * unitSize; /* calc point in range */
    if (betterTs < firstVirtualTs)
    {
        betterTs += intervalT;
    }

    return betterTs;
}

/* Monitor stuff */

static void gpBle_MonitorOpenConnection(Ble_IntConnId_t connId)
{
    UInt8 index = BLE_CONNID_TO_INDEX(connId);

    if(connId != BLE_ACTIVITY_MANAGER_ACTIVITY_ID_ADVERTISING && connId != GPBLEACTIVITYMANAGER_ACTIVITY_ID_INITIATING)
    {
        GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));
        GP_ASSERT_DEV_INT(Ble_MonitorLinkContext[index].activityId == BLE_CONN_HANDLE_INVALID);
        Ble_MonitorLinkContext[index].activityId = connId;
        Ble_MonitorLinkContext[index].connectionEstablished = false;
        Ble_MonitorLinkContext[index].slaveNbrConsecutiveLargeCorrWindows = 0;
    }
}

Bool BleActivityManager_GetMonitorContext(Ble_IntConnId_t connId, Ble_MonitorLinkContext_t* pMonitorContext)
{
    UInt8 index = BLE_CONNID_TO_INDEX(connId);

    if(index < number_of_elements(Ble_MonitorLinkContext))
    {
        MEMCPY(pMonitorContext, &Ble_MonitorLinkContext[index], sizeof(Ble_MonitorLinkContext_t));
        return true;
    }

    return false;
}
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

void BleActivityManager_UpdateMonitorContext(Ble_MonitorLinkContext_t* pMonitorContext)
{
    UInt8 index = BLE_CONNID_TO_INDEX(pMonitorContext->activityId);

    if(index < number_of_elements(Ble_MonitorLinkContext))
    {
        MEMCPY(&Ble_MonitorLinkContext[index], pMonitorContext, sizeof(Ble_MonitorLinkContext_t));
    }
}


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleActivityManager_Init(gpHal_BleCallbacks_t* pCallbacks)
{
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
    gpBleActivityManager_InitCommon();
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL
}

UInt16 gpBleActivityManager_GetPreferredInterval(void)
{
    return 0;
}

void gpBleActivityManager_Reset(Bool firstReset)
{
    UIntLoop i;

    // Reset link context
    for(i = 0; i < BLE_ACTIVITY_MANAGER_NUMBER_OF_ACTIVITIES; i++)
    {
        Ble_MonitorLinkContext[i].activityId = GPBLEACTIVITYMANAGER_ACTIVITY_ID_INVALID;
    }


#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
    gpBleActivityManager_ResetCommon();
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL
}

gpHci_Result_t gpBleActivityManager_StartAdvertising(gpBleActivityManager_IntervalMinMax_t* pInterval, gpBleActivityManager_Timing_t* pTiming, Bool highDutyCycleDirected, Bool singleEvent)
{
    BleActivityManager_GetStartTimeDummy(&pTiming->firstActivityTs);
    pTiming->interval = pInterval->intervalMax;

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBleActivityManager_StartScanning(gpBleActivityManager_ScanInput_t* pScanIntParams, gpBleActivityManager_ScanOutput_t* pScanTimingParams)
{
    BleActivityManager_GetStartTimeDummy(&pScanTimingParams->firstScanTs);

    return gpHci_ResultSuccess;
}

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
gpHci_Result_t gpBleActivityManager_StartInitScanning(gpBleActivityManager_ScanInput_t* pParams, gpBleActivityManager_Timing_t* pTiming)
{
    BleActivityManager_GetStartTimeDummy(&pTiming->firstActivityTs);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBleActivityManager_AddVirtualConnection(Ble_IntConnId_t connId, gpBleActivityManager_IntervalMinMax_t* pInterval, gpBleActivityManager_Timing_t* pTiming)
{
    pTiming->interval = pInterval->intervalMax;
    pTiming->firstActivityTs = BleActivityManager_CalculateMasterAnchorPoint(connId, pTiming->interval);

    gpBle_MonitorOpenConnection(connId);

    return gpHci_ResultSuccess;
}

void gpBleActivityManager_AddIncomingActivity(Ble_IntConnId_t activityId, UInt16 interval, UInt16 durationUnit625, UInt32 firstActivityTs)
{
    if(RANGE_CHECK(activityId, GPBLEACTIVITYMANAGER_ACTIVITY_ID_SYNC_START, GPBLEACTIVITYMANAGER_ACTIVITY_ID_SYNC_END))
    {
        // Nothing to be done when adding a sync
        return;
    }

    gpBle_MonitorOpenConnection(activityId);
}

void gpBleActivityManager_UpdateSlaveConnection(Ble_IntConnId_t connId, UInt16 interval)
{

}

// Called by master when sending CONN_UPDATE_IND
gpHci_Result_t gpBleActivityManager_UpdateMasterConnection(Ble_IntConnId_t connId, gpBleActivityManager_ConnUpdateInput_t* pIn, gpBleActivityManager_Timing_t* pActivityTiming)
{
    pActivityTiming->interval = pIn->intervalMax;

    return gpHci_ResultSuccess;
}

/* Approach for phase 1:
 * - pick one interval out of min and max, and use this to populate the conn param req (so min = interval = max)
 * - pick only one offset
 */
 // Called before constructing connection parameter request/response pdus
gpHci_Result_t gpBleActivityManager_GetPreferredAnchorPoint(Ble_IntConnId_t connId, gpBleActivityManager_ConnUpdateInput_t* pIn, gpBleActivityManager_Timing_t* pAnchor)
{
    UInt32 nextAnchor;
    gpHal_Result_t halResult;
    UInt8 randomOffset;
    UInt16 tmpInterval;

    halResult = gpHal_BleGetNextExpectedEventTimestamp(connId, &nextAnchor);
    GP_ASSERT_DEV_INT(halResult == gpHal_ResultSuccess);

   tmpInterval = pIn->intervalMin;

    pAnchor->firstActivityTs = nextAnchor + (pIn->currentLatency + BLE_ACTIVITY_MANAGER_MIN_LISTENING_INTERVALS + BLE_ACTIVITY_MANAGER_INTERVAL_SAFETY_MARGIN)*pIn->currentInterval;

    if(pIn->preferredPeriodicity == 0)
    {
        return gpHci_ResultSuccess;
    }

    while((tmpInterval % pIn->preferredPeriodicity) != 0)
    {
        tmpInterval++;
    }

    if(tmpInterval > pIn->intervalMax)
    {
        tmpInterval = pIn->intervalMax;
    }

    // Apply a random offset
    gpRandom_GetNewSequence(1, &randomOffset);

    while(randomOffset > tmpInterval)
    {
        randomOffset -= tmpInterval;
    }

    pAnchor->firstActivityTs += BLE_TIME_UNIT_1250_TO_US(randomOffset);

    return gpHci_ResultSuccess;
}

void gpBleActivityManager_CloseConnection(Ble_IntConnId_t connId)
{
    UInt8 index = BLE_CONNID_TO_INDEX(connId);

    if(connId != BLE_ACTIVITY_MANAGER_ACTIVITY_ID_ADVERTISING && connId != GPBLEACTIVITYMANAGER_ACTIVITY_ID_INITIATING)
    {
        if(!BLE_IS_INT_CONN_HANDLE_VALID(connId))
        {
            GP_LOG_SYSTEM_PRINTF("AM: error connId %u not valid",0, connId);
            GP_ASSERT_DEV_INT(false);
        }

        if(Ble_MonitorLinkContext[index].activityId != connId)
        {
            GP_LOG_SYSTEM_PRINTF("AM: invalid context for connId: %u (index: %u)",0, connId, index);

            if(index < BLE_ACTIVITY_MANAGER_NUMBER_OF_ACTIVITIES)
            {
                GP_LOG_SYSTEM_PRINTF("AM: conn id in context: %u",0, Ble_MonitorLinkContext[index].activityId);
            }
            GP_ASSERT_DEV_INT(false);
        }
        gpBleActivityManager_OnCloseConnectionCommon(connId);
        Ble_MonitorLinkContext[index].activityId = BLE_CONN_HANDLE_INVALID;
        Ble_MonitorLinkContext[index].connectionEstablished = false;
    }
}

UInt32 gpBleActivityManager_GetNextActivityTs(Ble_ActivityId_t activityId)
{
    UInt32 activityTs;

    {
        BleActivityManager_GetStartTimeDummy(&activityTs);
    }

    return activityTs;
}

// Callback function
void gpBleActivityManager_ConnectionEventDone(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;
    gpHal_ConnEventMetrics_t metrics;
    BleActivityManager_EventProcessed_t eventProcessedContext;

    pContext = Ble_GetLinkContext(connId);
    GP_ASSERT_DEV_INT(pContext != NULL);

    gpHal_BleGetConnectionMetrics(pContext->connId, &metrics);

    gpBleActivityManager_GetEventProcessedContext(pContext->hciHandle, &eventProcessedContext);

    gpBleActivityManager_ConnectionEventDoneCommon(&eventProcessedContext, &metrics);
}


void gpBleActivityManager_EstablishMasterConnection(Ble_IntConnId_t virtualConnId, Ble_IntConnId_t masterConnId, UInt32 currentEventTimeUs)
{
    UInt8 index = BLE_CONNID_TO_INDEX(virtualConnId);

    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(virtualConnId));
    GP_ASSERT_DEV_INT(virtualConnId & GPHAL_BLE_VIRTUAL_CONN_MASK);
    GP_ASSERT_DEV_INT(Ble_MonitorLinkContext[index].activityId != BLE_CONN_HANDLE_INVALID);
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(masterConnId));
    GP_ASSERT_DEV_INT((masterConnId & GPHAL_BLE_VIRTUAL_CONN_MASK) == 0);
    GP_ASSERT_DEV_INT(Ble_MonitorLinkContext[masterConnId].activityId == BLE_CONN_HANDLE_INVALID);

    // /* Store master connection context */
    Ble_MonitorLinkContext[masterConnId].activityId = masterConnId;
    Ble_MonitorLinkContext[masterConnId].connectionEstablished = true;
    Ble_MonitorLinkContext[masterConnId].slaveNbrConsecutiveLargeCorrWindows = Ble_MonitorLinkContext[index].slaveNbrConsecutiveLargeCorrWindows;

    // /* Clear virtual connection context */
    Ble_MonitorLinkContext[index].activityId = BLE_CONN_HANDLE_INVALID;
    Ble_MonitorLinkContext[index].connectionEstablished = false;
    Ble_MonitorLinkContext[index].slaveNbrConsecutiveLargeCorrWindows = 0;

    NOT_USED(currentEventTimeUs);
}
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

UInt8 gpBleActivityManager_GetNrOfActivities(void)
{
    return 0;
}

gpBleActivityManager_CollisionResult_t gpBleActivityManager_CheckForCollision(UInt32 *proposedDelay, UInt32 windowStart, UInt32 windowEnd)
{
    // stub - no implementation needed
    return gpBleActivityManager_NoCollision;
}

Bool gpBleActivityManager_IsActivityRegistered(Ble_ActivityId_t activityId)
{
    UIntLoop i;

    if(activityId == GPBLEACTIVITYMANAGER_ACTIVITY_ID_INVALID)
    {
        return false;
    }

    for(i = 0; i < BLE_ACTIVITY_MANAGER_NUMBER_OF_ACTIVITIES; i++)
    {
        if(Ble_MonitorLinkContext[i].activityId == activityId)
        {
            return true;
        }
    }

    return false;
}

void gpBleActivityManager_UnregisterActivity(Ble_ActivityId_t activityId)
{
    NOT_USED(activityId);
}
