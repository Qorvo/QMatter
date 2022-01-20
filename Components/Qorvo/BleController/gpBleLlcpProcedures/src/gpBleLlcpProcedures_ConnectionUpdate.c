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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleLlcpProcedures/src/gpBleLlcpProcedures_ConnectionUpdate.c#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLELLCPPROCEDURES

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleConfig.h"
#include "gpBleDataCommon.h"
#include "gpBleDataChannelTxQueue.h"
#include "gpBleInitiator.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcpProcedures_defs.h"
#include "gpBleLlcpProcedures_Update_defs.h"
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpSched.h"
#include "gpBleActivityManager.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_REF_CONN_EVENT_COUNT_OFFSET     20

#define BLE_CONN_UPDATE_PREFERRED_PERIODICITY   7

// PDU payload lengths
#define BLE_CTR_DATA_LENGTH_CONN_UPDATE_IND 11
#define BLE_CTR_DATA_LENGTH_CONN_PARAM_REQ_RSP  23

#define BLE_CONN_PARAM_REQ_NR_OF_OFFSETS            6

// How long it takes to transmit a packet with 27 payload bytes (and a MIC) on S=8 coding
#define BLE_CODED_PHY_S8_27_BYTES_DURATION_US       2704

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_INIT_CONNECTION_UPDATE_PDU_DESCRIPTOR()         {gpBleLlcp_OpcodeConnectionUpdateInd, 11, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_CENTRAL}

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
*****************************************************************************/

// checker/action functions
static gpHci_Result_t Ble_ConnectionParametersChecker(gpHci_ConnectionHandle_t connHandle, UInt16 intervalMin, UInt16 intervalMax, UInt16 latency, UInt16 timeout, UInt16 minCe, UInt16 maxCe);
static gpHci_Result_t Ble_ConnectionUpdateAction(Ble_IntConnId_t connId, gpHci_LeConnectionUpdateCommand_t* pUpdateData);


// Connection update procedure
#ifdef GP_DIVERSITY_BLE_MASTER
static void Ble_LlcpConnectionUpdateGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData);
#endif
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdateStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdatePduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdatePduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static void Ble_LlcpConnectionUpdateFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpRegisterConnectionUpdateProcedure(void);

// Connection parameters request

// Various
#ifdef GP_DIVERSITY_BLE_MASTER
static void Ble_LlcpConnectionUpdateIndGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, UInt8* pCtrDataLength, UInt8* pCtrData, Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu);
#endif //GP_DIVERSITY_BLE_MASTER

static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdateReqStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static void Ble_LlcpConnectionUpdateFinishedCommon(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static gpHci_Result_t Ble_LlcpTriggerOptimalUpdateProcedure(Ble_LlcpLinkContext_t* pContext, Ble_LlcpConnParamReqRspData_t* pProcedureData);
#ifdef GP_DIVERSITY_BLE_MASTER
static void Ble_PopulateUpdateData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu, gpBleActivityManager_Timing_t* pTiming, const gpHal_ConnEventMetrics_t *metrics);
#endif //GP_DIVERSITY_BLE_MASTER
static void BleLlcp_ReverseCalculateInstantTs(Ble_LlcpLinkContext_t * pContext, UInt32 *out_instantTs, UInt32 *out_lastCorrelationTs, UInt32 nextAnchorTs, UInt32 lastCorrelationTs, UInt16 instantOffset);


static UInt8 BleLLcpProcedures_GetPreferredPeriodicity(UInt16 intervalMax);

static void BleLlcpProcedures_ScheduleUpdateCompleteEvent(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);

static inline UInt8 BleLlcpProcedures_UpdateUnmappedChannelPtr(Ble_LlcpLinkContext_t* pContext, UInt8 unmappedChannelPtr, UInt16 eventCountOffset);

/*****************************************************************************
 *                    Procedure descriptor
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_ConnectionUpdatePduDescriptors[] =
{
    BLE_INIT_CONNECTION_UPDATE_PDU_DESCRIPTOR()
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_ConnectionUpdateDescriptor =
{
    .procedureFlags = GPBLELLCPFRAMEWORK_PROCEDURE_FLAGS_INSTANT_BM,
    .procedureDataLength = sizeof(Ble_LlcpConnParamReqRspData_t),
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_ConnectionUpdatePduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_ConnectionUpdatePduDescriptors,
    .featureMask = GPBLELLCP_FEATUREMASK_NONE,
    .cbQueueingNeeded = NULL,
    .cbProcedureStart = NULL,
#ifdef GP_DIVERSITY_BLE_MASTER
    .cbGetCtrData = Ble_LlcpConnectionUpdateGetCtrData,
#endif // GP_DIVERSITY_BLE_MASTER
    .cbStoreCtrData = Ble_LlcpConnectionUpdateStoreCtrData,
    .cbPduReceived = Ble_LlcpConnectionUpdatePduReceived,
    .cbUnexpectedPduReceived = NULL,
    .cbPduQueued = NULL,
    .cbPduTransmitted = Ble_LlcpConnectionUpdatePduTransmitted,
    .cbFinished = Ble_LlcpConnectionUpdateFinished,
};


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

gpHci_Result_t Ble_ConnectionParametersChecker(gpHci_ConnectionHandle_t connHandle, UInt16 intervalMin, UInt16 intervalMax, UInt16 latency, UInt16 timeout, UInt16 minCe, UInt16 maxCe)
{
    if(gpBleLlcp_IsHostConnectionHandleValid(connHandle) != gpHci_ResultSuccess)
    {
        GP_LOG_PRINTF("Unknown conn id: %x", 0, connHandle);
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    if(!Ble_ConnectionParametersValid(intervalMin, intervalMax, latency, timeout, minCe, maxCe))
    {
        return gpHci_ResultInvalidHCICommandParameters;
    }

    return gpHci_ResultSuccess;
}

gpHci_Result_t Ble_ConnectionUpdateAction(Ble_IntConnId_t connId, gpHci_LeConnectionUpdateCommand_t* pConnUpdate)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpConnParamReqRspData_t updateData;

    pContext = Ble_GetLinkContext(connId);
    GP_ASSERT_DEV_INT(pContext);

    MEMSET(&updateData, 0, sizeof(Ble_LlcpConnParamReqRspData_t));
    updateData.pdu.intervalMin = pConnUpdate->connIntervalMin;
    updateData.pdu.intervalMax = pConnUpdate->connIntervalMax;
    updateData.pdu.timeout = pConnUpdate->supervisionTimeout;
    updateData.pdu.latency = pConnUpdate->connLatency;
    updateData.pdu.preferredPeriodicity = BleLLcpProcedures_GetPreferredPeriodicity(pConnUpdate->connIntervalMax);
    updateData.minCELength = pConnUpdate->minCELength;
    updateData.maxCELength = pConnUpdate->maxCELength;
    MEMSET(updateData.pdu.offsets, 0xFF, sizeof(updateData.pdu.offsets));

    return Ble_LlcpTriggerOptimalUpdateProcedure(pContext, &updateData);
}


//-----------------------------
// Connection Update procedure
//-----------------------------

#ifdef GP_DIVERSITY_BLE_MASTER
void Ble_LlcpConnectionUpdateGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData)
{
    Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    Ble_LlcpConnectionUpdateIndGetCtrData(pContext, pProcedure, pCtrDataLength, pCtrData, pConnUpdatePdu);
}
#endif // GP_DIVERSITY_BLE_MASTER

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdateStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    // We received an LL_CONNECTION_UPDATE_IND - and the connection parameter negotiation procedure was not run before
    // use the current values of the maxCELength and minCELength
    pConnUpdatePdu->minCELength = pContext->ccParams.minCELength;
    pConnUpdatePdu->maxCELength = pContext->ccParams.maxCELength;

    return Ble_LlcpConnectionUpdateReqStoreCtrData(pContext, pProcedure, pPdLoh, opcode);
}

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdatePduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodeConnectionUpdateInd:
        {
            action = Ble_LlcpFrameworkActionPause;
            break;
        }
        case gpBleLlcp_OpcodeUnknownRsp:
        {
            action = Ble_LlcpFrameworkActionStop;
            break;
        }
        default:
        {
            // Should not happen
            GP_ASSERT_DEV_INT(false);
        }
    }
    // We need to suspend execution untill the configured instant occurs
    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdatePduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu;

    GP_ASSERT_DEV_INT(rxOpcode == gpBleLlcp_OpcodeConnectionUpdateInd);

    pConnUpdatePdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    gpBleActivityManager_UpdateSlaveConnection(pContext->connId, pConnUpdatePdu->interval);

    pProcedure->currentTxPdu = gpBleLlcp_OpcodeInvalid;
    pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;

    // We need to suspend execution untill the configured instant occurs
    return Ble_LlcpFrameworkActionPause;
}

void Ble_LlcpConnectionUpdateFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    Ble_LlcpConnectionUpdateFinishedCommon(pContext, pProcedure, notifyHost);
}

static inline UInt8 BleLlcpProcedures_UpdateUnmappedChannelPtr(Ble_LlcpLinkContext_t* pContext, UInt8 unmappedChannelPtr, UInt16 eventCountOffset)
{
    // The current UnmappedChannelPtr has a value < 37, hopIncrement has a max value of 16  and eventCountOffset is UInt16
    // We calculate in UInt32 so we don't have to worry about overflows

    UInt32 updatedUnmappedChannelPtr = (UInt32)unmappedChannelPtr;
    UInt8 hopIncrement = gpHal_BleGetHopIncrement(pContext->connId);

    if(!BLE_RANGE_CHECK(hopIncrement, BLE_INITIATOR_HOP_FIELD_MIN, BLE_INITIATOR_HOP_FIELD_MAX) || (unmappedChannelPtr >= BLE_DATA_NUMBER_OF_CHANNELS))
    {
        // Return here in case of invalid parameters
        GP_ASSERT_DEV_INT(false);
        return (UInt8)updatedUnmappedChannelPtr;
    }

    /* Note that:
     *     (                            -hopIncrement  * eventCountOffset ) % BLE_DATA_NUMBER_OF_CHANNELS
     *  == ((BLE_DATA_NUMBER_OF_CHANNELS-hopIncrement) * eventCountOffset ) % BLE_DATA_NUMBER_OF_CHANNELS
      */
    updatedUnmappedChannelPtr += (UInt32)(BLE_DATA_NUMBER_OF_CHANNELS-hopIncrement)*eventCountOffset;
    updatedUnmappedChannelPtr %= BLE_DATA_NUMBER_OF_CHANNELS;
    return (UInt8)updatedUnmappedChannelPtr;
}

void Ble_LlcpConnectionUpdateInstantPassed(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    gpHal_Result_t result;

    gpHal_UpdateConnEventInfo_t updateInfo;
    Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu;
    gpHal_ConnEventMetrics_t connMetrics;
    UInt32 instantTs;
    UInt32 lastCorrelationTs;
    UInt32 nextConnTs;
    UInt32 initialWindowSize = 0;
    UInt8 unmappedChannelPtr = 0;
    UInt16 eventCountOffset = 0;
    UInt16 nrNoRXEvents = 0;

    // The connection's event schedule has been stopped by the RT subsystem (BLE Event Mgr)
    // so it is safe to read
    result = gpHal_BleGetConnectionMetrics(pContext->connId, &connMetrics);

    if (result != gpHal_ResultSuccess)
    {
        // Update failed ==> terminate connection ?
        // . . . input parameter (connId) was incorrect, so we do not have a hook to terminate the connection
        // probably the connection has terminated already
        // We can probably create a connection update scenario where the LSTO expires just before when the update instant occurs . . .
        // : do we need the ASSERT here ?
        GP_ASSERT_DEV_INT(false);
        return;
    }

    result = gpHal_BleGetUnmappedChannelPtr(pContext->connId, &unmappedChannelPtr);

    if (result != gpHal_ResultSuccess)
    {
        // FixMe : same error handling as for gpHal_BleGetNextExpectedEventTimestamp
        return;
    }

    pConnUpdatePdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    GP_LOG_PRINTF("UIP: instant=%u ec=%u",0,pConnUpdatePdu->instant,connMetrics.eventCounterNext);
    /* Note that the eventcounter was incremented for next event already by RT */
    GP_ASSERT_DEV_INT( pConnUpdatePdu->instant <= connMetrics.eventCounterNext);
    /* The difference between the current event count and the instant */
    eventCountOffset = BLE_GET_EC_DIFF(pConnUpdatePdu->instant, connMetrics.eventCounterNext);

    BleLlcp_ReverseCalculateInstantTs(pContext, &instantTs, &lastCorrelationTs, connMetrics.nextAnchorTime, connMetrics.tsLastPacketReceived, eventCountOffset);

    if(pContext->masterConnection)
    {
        // As a master, we determine ourselves what the exact anchor time is
        nextConnTs = pConnUpdatePdu->firstAnchorPoint;

        // Note that initialWindowSize == 0 is OK for master
        initialWindowSize = 0;
    }
    else
    {
        UInt32 noRxTime;

        noRxTime = BLE_GET_TIME_DIFF(lastCorrelationTs,instantTs);

        /* We only calculate the RX window at the instant and let the RT system catch up
         * OPTIMIZE: update the event count up to the current time here in NRT */
        nextConnTs =
            gpBleLlcp_CalculateEarliestAnchorPoint(
                pContext->connId,                                       /* connection identifier */
                lastCorrelationTs,                                      /* last correlation TS */
                noRxTime,                                               /* blackout window after end of Rx */
                BLE_TIME_UNIT_1250_TO_US(pConnUpdatePdu->winOffset),    /* new window offset */
                BLE_TIME_UNIT_1250_TO_US(pConnUpdatePdu->winSize),      /* new TX window size */
                &initialWindowSize                                      /* RX window duration */
            );
    }

    // Populate the update information for the gpHal
    updateInfo.interval = BLE_TIME_UNIT_1250_TO_US(pConnUpdatePdu->interval);
#ifdef GP_DIVERSITY_DEVELOPMENT
    updateInfo.interval += Ble_GetVsdArtificialDriftAsSignedNbrMicrosec();
#endif /* GP_DIVERSITY_DEVELOPMENT */

    /* Do not activate slave latency here, needs to go through gpBleLlcp_UpdateSlaveLatency for that. */
    updateInfo.latency = 0;
    updateInfo.windowDuration = initialWindowSize;
    updateInfo.eventCount = pConnUpdatePdu->instant;
    // Correct the unmappedChannelPtr with the nbr of missed intervals for which we were too late
    updateInfo.unmappedChannelPtr = BleLlcpProcedures_UpdateUnmappedChannelPtr(pContext, unmappedChannelPtr, eventCountOffset);
    /* The spec (4.2 [Vol6, PartB] 5.1.1) says to reset the supervision timer at the instant */
    updateInfo.tsLastValidPacketReceived = instantTs;
    nrNoRXEvents = connMetrics.nrNoRXEvents;
    /* This is used for slave latency (nr_no_rx_events < slave_latency). We correct the old error. */
    updateInfo.nrNoRXEvents = nrNoRXEvents < eventCountOffset ? 0 : nrNoRXEvents - eventCountOffset;


    result = gpHal_UpdateConnection(pContext->connId, nextConnTs, &updateInfo);

    if(result != gpHal_ResultSuccess)
    {
        // Update failed ==> terminate connection ?
        GP_ASSERT_DEV_INT(false);
        return;
    }
}

void Ble_LlcpRegisterConnectionUpdateProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdConnectionUpdate, &BleLlcpProcedures_ConnectionUpdateDescriptor);

    gpBleLlcpFramework_RegisterInvalidProcedureAction(gpBleLlcp_ProcedureIdConnectionUpdate, false);
}


#ifdef GP_DIVERSITY_BLE_MASTER
void Ble_LlcpConnectionUpdateIndGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, UInt8* pCtrDataLength, UInt8* pCtrData, Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu)
{
    UInt8 index = 0;
    gpHal_ConnEventMetrics_t metrics;

    gpBleActivityManager_Timing_t activityTiming;


    // No forced anchor move when anchormoveOffset = 0
    if(pConnUpdatePdu->anchorMoveOffset == 0)
    {
        gpBleActivityManager_ConnUpdateInput_t smInput;

        // Request new connection parameters to the BLE activity manager
        smInput.intervalMin = pConnUpdatePdu->pdu.intervalMin;
        smInput.intervalMax = pConnUpdatePdu->pdu.intervalMax;
        smInput.preferredPeriodicity = pConnUpdatePdu->pdu.preferredPeriodicity;

        gpBleActivityManager_UpdateMasterConnection(pContext->connId, &smInput, &activityTiming);

        GP_LOG_PRINTF("conn upd interval us = %lu (range = [%lu-%lu])",0,
            (unsigned long)BLE_TIME_UNIT_1250_TO_US(activityTiming.interval),
            (unsigned long)BLE_TIME_UNIT_1250_TO_US(smInput.intervalMin),
            (unsigned long)BLE_TIME_UNIT_1250_TO_US(smInput.intervalMax)
        );
        GP_LOG_PRINTF("conn upd interval units = %u (range = [%u-%u])",0,
            (activityTiming.interval),
            (smInput.intervalMin),
            (smInput.intervalMax)
        );
    }
    else
    {
        // Forced anchor move, honor the request
        activityTiming.firstActivityTs = gpBleActivityManager_GetNextActivityTs(pContext->connId);
        activityTiming.firstActivityTs += BLE_TIME_UNIT_1250_TO_US(pConnUpdatePdu->anchorMoveOffset);
        activityTiming.interval = pConnUpdatePdu->pdu.intervalMin;
    }

    gpHal_BleGetConnectionMetrics(pContext->connId, &metrics);
    Ble_PopulateUpdateData(pContext, pConnUpdatePdu, &activityTiming, &metrics);

    gpBle_AppendWithUpdate(&pCtrData[index], &pConnUpdatePdu->winSize, &index, 1);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pConnUpdatePdu->winOffset, &index, 2);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&activityTiming.interval, &index, 2);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pConnUpdatePdu->pdu.latency, &index, 2);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pConnUpdatePdu->pdu.timeout, &index, 2);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pConnUpdatePdu->instant, &index, 2);

    *pCtrDataLength = index;

    Ble_LlcpConfigureLastScheduledConnEventAfterCurrent(pProcedure, metrics.eventCounterNext, pConnUpdatePdu->instant);
}
#endif //GP_DIVERSITY_BLE_MASTER

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdateReqStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    Ble_LlcpConnParamReqRspData_t* pConnUpdate;
    UInt16 pduConnEventCount;

    pConnUpdate = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    GP_ASSERT_DEV_INT(pConnUpdate != NULL);

    gpPd_ReadWithUpdate(pPdLoh, 1, &pConnUpdate->winSize);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pConnUpdate->winOffset);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pConnUpdate->interval);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pConnUpdate->pdu.latency);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pConnUpdate->pdu.timeout);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pConnUpdate->instant);

    if(!Ble_ConnectionParametersValid(pConnUpdate->interval, pConnUpdate->interval, pConnUpdate->pdu.latency, pConnUpdate->pdu.timeout, 0, 0))
    {
        pProcedure->result = gpHci_ResultInvalidLMPParametersInvalidLLParameters;
        return Ble_LlcpFrameworkActionRejectWithUnknownRsp;
    }

    if(!Ble_LlcpTxWinOffsetValid(pConnUpdate->winOffset, pConnUpdate->interval))
    {
        // If window offset is invalid, we do not apply the update
        pProcedure->result = gpHci_ResultInvalidLMPParametersInvalidLLParameters;
        return Ble_LlcpFrameworkActionRejectWithUnknownRsp;
    }

    if(!Ble_LlcpTxWinSizeValid(pConnUpdate->winSize, pConnUpdate->interval))
    {
        // Correct invalid winSize to max valid winSize
        pConnUpdate->winSize = BLE_LL_DATA_WIN_SIZE_UPPER;
    }

    pduConnEventCount = Ble_LlcpGetPduConnEventCount(pContext, pPdLoh);

    if(!Ble_LlcpInstantValid(pConnUpdate->instant, pduConnEventCount))
    {
        GP_LOG_PRINTF("Instant is in the past, connection lost",0);
        // Schedule to make sure we do not terminate while procedure still running
        gpSched_ScheduleEventArg(0, Ble_LlcpInstantInvalidFollowUp, pContext);
        return Ble_LlcpFrameworkActionContinue;
    }

    // configure the instant-
    Ble_LlcpConfigureLastScheduledConnEventAfterPassed(pProcedure, pduConnEventCount, pConnUpdate->instant);

    return Ble_LlcpFrameworkActionContinue;
}


//--------
// Various
//--------


void Ble_LlcpConnectionUpdateFinishedCommon(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu;
    Bool wasAnchorMove;

    pConnUpdatePdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    NOT_USED(notifyHost);

    if(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_INTERRUPTED_BY_TERMINATION_IDX))
    {
        Ble_LlcpStopLastScheduledConnEventCount(pContext);
        return;
    }


    if(pProcedure->procedureId == gpBleLlcp_ProcedureIdConnectionParamRequest && pProcedure->result == gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature)
    {
        // Remote does not support connection parameter request, try with connection update when we are master
        if(pContext->masterConnection && pProcedure->localInit)
        {
            gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

            MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
            startDescriptor.procedureId = gpBleLlcp_ProcedureIdConnectionUpdate;
            startDescriptor.controllerInit = pProcedure->controllerInit;
            MEMCPY(&startDescriptor.procedureData, pConnUpdatePdu, sizeof(Ble_LlcpConnParamReqRspData_t));

            gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
            return;
        }
    }

    if(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX))
    {
        return;
    }

    wasAnchorMove = Ble_LlcpAnchorMoveRequested(pContext->connId, pConnUpdatePdu->interval, pConnUpdatePdu->pdu.latency, pConnUpdatePdu->pdu.timeout);

    if(pProcedure->result == gpHci_ResultSuccess)
    {
        // Store the new connection parameters in case the procedure was successful
        pContext->intervalUnit = pConnUpdatePdu->interval;
        pContext->timeoutUnit = pConnUpdatePdu->pdu.timeout;
        pContext->ccParams.minCELength = pConnUpdatePdu->minCELength;
        pContext->ccParams.maxCELength = pConnUpdatePdu->maxCELength;

        // Use function iso writing slave latency directly as this can have side-effects (SDP004-285).
        gpBleLlcp_UpdateSlaveLatency(pContext->connId, pConnUpdatePdu->pdu.latency);

        /* Stop supervision timeout
         * The spec (4.2 [Vol6, PartB] 5.1.1) says that we need to reset the supervision timeout on the instant.
         * As we do not have any callback on the instant, we stop the supervision timeout here, and reschedule it on
         * the first connection event after the instant.
         */
        gpBleActivityManager_StopSupervisionTimeout(pContext->connId);

        // Note that pConnUpdatePdu->max/minCELength contains the value requested by the Host (if available)
        //      or the current stored value in case no (recent) Host value available
        // So we can safely use pConnUpdatePdu->max/minCELength and call gpBle_SetConnectionBandwidthControl unconditionally.
        // Notes:
        // - pProcedure->controllerInit is only set to false in case the Host has initiated the procedure
        //   So, pProcedure->controllerInit is set to true if it is a remote initiated update,
        //   even if there has been a HCI Remote Connection Parameter Request event + HCI Remote Connection Parameter Request Reply command
        // - Ble_LlcpAnchorMoveRequested indicates whether connection parameters (interval, latency and LSTO) are unchanged
        // In short: it is safe to always call gpBle_SetConnectionBandwidthControl: input values are always valid
        gpBle_SetConnectionBandwidthControl(pContext->connId, pConnUpdatePdu->maxCELength, pConnUpdatePdu->interval);
    }

    if(!pProcedure->controllerInit || !wasAnchorMove)
    {
        BleLlcpProcedures_ScheduleUpdateCompleteEvent(pContext, pProcedure);
    }
}

gpHci_Result_t Ble_LlcpTriggerOptimalUpdateProcedure(Ble_LlcpLinkContext_t* pContext, Ble_LlcpConnParamReqRspData_t* pProcedureData)
{
    gpHci_Result_t result = gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature;
    Bool anchorMove = false;
    gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

    MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
    MEMCPY(&startDescriptor.procedureData, pProcedureData, sizeof(Ble_LlcpConnParamReqRspData_t));

    if((pProcedureData->pdu.intervalMin == pProcedureData->pdu.intervalMax) &&
        Ble_LlcpAnchorMoveRequested(pContext->connId, pProcedureData->pdu.intervalMin, pProcedureData->pdu.latency, pProcedureData->pdu.timeout))
    {
        anchorMove = true;
    }

    if(pContext->masterConnection && anchorMove)
    {
        startDescriptor.procedureId = gpBleLlcp_ProcedureIdConnectionUpdate;

        // No negotiation needed if anchor move is requested by the master
        return gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
    }


    if(pContext->masterConnection && (result == gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature))
    {
        startDescriptor.procedureId = gpBleLlcp_ProcedureIdConnectionUpdate;
        // On a master, use connection update procedure if one (or both) sides do not support the connections parameter request procedure
        result = gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
    }

    return result;
}


#ifdef GP_DIVERSITY_BLE_MASTER
void Ble_PopulateUpdateData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu, gpBleActivityManager_Timing_t* pTiming, const gpHal_ConnEventMetrics_t *metrics)
{
    UInt16 intervalDiff;
    UInt32 firstActivityTs;
    UInt32 instantTime;
    UInt32 intervalUs;
    UInt32 intervalNewUs;
    UInt32 timeDiff;
    UInt16 eventCounter;
    UInt16 i;

    eventCounter = metrics->eventCounterNext;

    firstActivityTs = pTiming->firstActivityTs;
    pConnUpdatePdu->interval = pTiming->interval;
    pConnUpdatePdu->instant  = eventCounter + ((pContext->latency+1)*6) + 1;

    intervalUs = BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit);
    intervalNewUs = BLE_TIME_UNIT_1250_TO_US(pConnUpdatePdu->interval);

    // Calculate difference between current event count and instant
    if(pConnUpdatePdu->instant  > eventCounter)
    {
        intervalDiff = pConnUpdatePdu->instant  - eventCounter;
    }
    else
    {
        intervalDiff = eventCounter - pConnUpdatePdu->instant;
    }

    instantTime = metrics->nextAnchorTime;

    // Calculate the (theoretical) timestamp of the instant
    for(i = 0; i < intervalDiff; i++)
    {
        instantTime += intervalUs;
    }

    // When activityTs > instantTime, the instant occurs after the wrap. Advance the activityTs after the wrap
    if(firstActivityTs > instantTime)
    {
        while(firstActivityTs < firstActivityTs + intervalNewUs)
        {
            firstActivityTs += intervalNewUs;
        }
        // This causes a wrap
        firstActivityTs += intervalNewUs;
    }

    // Advance the activity TS after the instant time
    while(firstActivityTs < instantTime)
    {
        firstActivityTs += intervalNewUs;
    }

    // Calculate difference between ideal time and instant time and use this to populate winOffset and winSize
    timeDiff = firstActivityTs - instantTime;

    // This should be made configurable (by using gpBle_VsdConnReqWinSize)
    pConnUpdatePdu->winSize = BLE_CONN_PDU_DEFAULT_WIN_SIZE;

    if(BLE_TIME_UNIT_1250_TO_US(pConnUpdatePdu->winSize) >= timeDiff)
    {
        pConnUpdatePdu->winOffset = 0;
    }
    else
    {
        pConnUpdatePdu->winOffset = BLE_US_TO_1250_TIME_UNIT(timeDiff - BLE_TIME_UNIT_1250_TO_US(pConnUpdatePdu->winSize)) + 1;
    }

    pConnUpdatePdu->firstAnchorPoint = firstActivityTs;
}
#endif //GP_DIVERSITY_BLE_MASTER

void BleLlcp_ReverseCalculateInstantTs(  Ble_LlcpLinkContext_t * pContext,
                                         UInt32 *out_instantTs,
                                         UInt32 *out_lastCorrelationTs,
                                         UInt32 nextAnchorTs,
                                         UInt32 lastCorrelationTs,
                                         UInt16 instantOffset)
{
    UInt32 instantTs = 0;
    UInt32 nextAnchorDiff = 0;
    UInt32 noRxTime = 0;
    UInt16 intervalUnit = pContext->intervalUnit;

    /* The difference between the current event count and the instant */
    nextAnchorDiff = instantOffset * BLE_TIME_UNIT_1250_TO_US(intervalUnit);
    instantTs = (UInt32)nextAnchorTs - nextAnchorDiff;

    GP_LOG_PRINTF("instantTs=%lu nextAnchorTs=%lu i:%u o:%u",0, (unsigned long)instantTs, (unsigned long)nextAnchorTs, intervalUnit, instantOffset);
    GP_LOG_PRINTF("lastCorrelationTs=%lu",0, (unsigned long)lastCorrelationTs);

    if(pContext->masterConnection)
    {
        /* Since the TX time does not deviate if a packet was not received we can simply calculate back */
        *out_instantTs = instantTs;
        *out_lastCorrelationTs = instantTs - BLE_TIME_UNIT_1250_TO_US(intervalUnit);
        return;
    }

    /* It is possible to that instantTs < lastCorrelationTs, regardless of overflow!!!
     * This means we received after the instant despite not having applied the new settings yet.
     * This is not as unlikely as you might think if the TX window size is large!
     *
     * We need to detect this:
     *    instantTs <= lastCorrelationTs (after overflow correct)
     *
     * Since we can have overflows this can't be solved with a trivial comparison. However, we can be sure that:
     *
     *    1)  lastCorrelationTs <= nextAnchorTs (after overflow correction)
     *    2)  instantTs <= nextAnchorTs (after overflow correction)
     *
     *   let d_lastRx  = BLE_GET_TIME_DIFF(lastCorrelationTs, nextAnchorTs)
     *   let d_instant = BLE_GET_TIME_DIFF(instantTs, nextAnchorTs)
     *
     *   Since BLE_GET_TIME_DIFF corrects for overflows we can be sure we received
     *   after the instant if (d_lastRx <= d_instant)
     */
    if ( BLE_GET_TIME_DIFF(lastCorrelationTs,nextAnchorTs) <= nextAnchorDiff )
    {
        /* We managed to sync after the instant */
        noRxTime = BLE_TIME_UNIT_1250_TO_US(intervalUnit);
        lastCorrelationTs = instantTs - noRxTime;
    }

    *out_instantTs = instantTs;
    *out_lastCorrelationTs = lastCorrelationTs;
}


UInt8 BleLLcpProcedures_GetPreferredPeriodicity(UInt16 intervalMax)
{

//     UInt16 amPreferredIntervalUnit;
//     UInt16 preferredPeriodicityUnit;

//     // Only checking the max interval might not be optimal, especially when interval min and max are not equal
//     // If we want to improve this, we should check if there is a better preference in the range of min and max interval.
//     amPreferredIntervalUnit = gpBleActivityManager_GetPreferredInterval() >> 1;
//     preferredPeriodicityUnit = gpBle_GetGcd(amPreferredIntervalUnit, intervalMax);

    return 0;
}

void BleLlcpProcedures_ScheduleUpdateCompleteEvent(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    // Notify host when the procedure was initiated by host, or when it was more than an anchor move
    gpHci_EventCbPayload_t payload;
    Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu;

    MEMSET(&payload, 0, sizeof(gpHci_EventCbPayload_t));
    pConnUpdatePdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    payload.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeConnectionUpdateComplete;
    payload.metaEventParams.params.connectionUpdateComplete.status = pProcedure->result;
    payload.metaEventParams.params.connectionUpdateComplete.connectionHandle = pContext->hciHandle;

    if(pProcedure->result == gpHci_ResultSuccess)
    {
        // Only add those parameters in case of success
        payload.metaEventParams.params.connectionUpdateComplete.connInterval = pConnUpdatePdu->interval;
        payload.metaEventParams.params.connectionUpdateComplete.connLatency = pConnUpdatePdu->pdu.latency;
        payload.metaEventParams.params.connectionUpdateComplete.supervisionTo = pConnUpdatePdu->pdu.timeout;
    }

    gpBle_ScheduleEvent(0, gpHci_EventCode_LEMeta, &payload);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_ConnectionUpdateInit(void)
{
    Ble_LlcpRegisterConnectionUpdateProcedure();
}

gpHci_Result_t gpBleLlcpProcedures_TriggerAnchorMove(Ble_IntConnId_t connId, UInt16 offset)
{
    Ble_LlcpConnParamReqRspData_t procedureData;
    Ble_LlcpLinkContext_t* pContext;

    GP_LOG_PRINTF("trigger anchor move %x %x",0, connId, offset);

    pContext = Ble_GetLinkContext(connId);

    if(pContext == NULL)
    {
        GP_LOG_PRINTF("inv params",0);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    // Populate procedure data: no change vs current connection parameters: only offset changes
    MEMSET(&procedureData, 0, sizeof(Ble_LlcpConnParamReqRspData_t));
    procedureData.anchorMoveOffset = offset;
    procedureData.pdu.intervalMin = pContext->intervalUnit;
    procedureData.pdu.intervalMax = pContext->intervalUnit;
    procedureData.pdu.preferredPeriodicity = BleLLcpProcedures_GetPreferredPeriodicity(pContext->intervalUnit);
    procedureData.pdu.timeout = pContext->timeoutUnit;
    procedureData.pdu.latency = pContext->latency;
    procedureData.minCELength = pContext->ccParams.minCELength;
    procedureData.maxCELength = pContext->ccParams.maxCELength;

    // Populate the new offset value for the anchor move
    // Note that the ref conn event count can have any value - since the connection interval does not change
    MEMSET(procedureData.pdu.offsets, 0xFF, sizeof(procedureData.pdu.offsets));
    procedureData.pdu.refConnEventCount = (gpHal_BleGetCurrentConnEventCount(pContext->connId) + 2) & 0xFFFF; // just a value in the future
    procedureData.pdu.offsets[0] = offset;

    return Ble_LlcpTriggerOptimalUpdateProcedure(pContext, &procedureData);
}

/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_LeConnectionUpdate(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_LeConnectionUpdateCommand_t* pConnUpdate;

    pConnUpdate = &pParams->LeConnectionUpdate;

    BLE_SET_RESPONSE_EVENT_COMMAND_STATUS(pEventBuf->eventCode);

    result = Ble_ConnectionParametersChecker(
        pConnUpdate->connectionHandle, pConnUpdate->connIntervalMin, pConnUpdate->connIntervalMax,
        pConnUpdate->connLatency, pConnUpdate->supervisionTimeout, pConnUpdate->minCELength, pConnUpdate->maxCELength
    );

    if(result == gpHci_ResultSuccess)
    {
        Ble_IntConnId_t connId;

        connId = gpBleLlcp_HciHandleToIntHandle(pConnUpdate->connectionHandle);
        result = Ble_ConnectionUpdateAction(connId, pConnUpdate);
    }

    return result;
}


