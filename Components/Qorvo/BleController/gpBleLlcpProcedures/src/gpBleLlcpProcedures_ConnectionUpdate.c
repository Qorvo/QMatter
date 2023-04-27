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

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
static gpHci_Result_t Ble_ConnectionParamReqReplyAction(Ble_IntConnId_t connId, gpHci_LeRemoteConnectionParamRequestReplyCommand_t* pRequest);
static gpHci_Result_t Ble_ConnectionParamReqNegReplyAction(Ble_IntConnId_t connId, gpHci_Result_t reason);
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

// Connection update procedure
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdateStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdatePduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdatePduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static void Ble_LlcpConnectionUpdateFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpRegisterConnectionUpdateProcedure(void);

// Connection parameters request
#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
static void Ble_LlcpConnectionParamReqGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqPduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqUnexpectedPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh);
static void Ble_LlcpConnectionParamReqFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpRegisterConnectionParamReqProcedure(void);
#endif // GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

// Various

static Ble_LlcpFrameworkAction_t Ble_LlcpConnectionUpdateReqStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
static void Ble_LlcpReadConnParamReqRspPdu(Ble_LlcpConnParamReqRspPdu_t* pPdu, gpPd_Loh_t* pPdLoh);
static void Ble_LLcpConnParamReqPduToBuffer(Ble_LlcpConnParamReqRspData_t* pPdu, UInt8* pCtrData, UInt8* pCtrDataLength);
static void Ble_LlcpPrepareConnectionParamReqRspData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpConnParamReqRspData_t* pConnReqPdu, UInt8* pCtrDataLength, UInt8* pCtrData);
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
static void Ble_LlcpConnectionUpdateFinishedCommon(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static gpHci_Result_t Ble_LlcpTriggerOptimalUpdateProcedure(Ble_LlcpLinkContext_t* pContext, Ble_LlcpConnParamReqRspData_t* pProcedureData);
#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
static Bool BleLlcpProcedures_ConnParamReqReplyCmdAllowed(Ble_LlcpProcedureContext_t* pProcedure);
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
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
    .cbStoreCtrData = Ble_LlcpConnectionUpdateStoreCtrData,
    .cbPduReceived = Ble_LlcpConnectionUpdatePduReceived,
    .cbUnexpectedPduReceived = NULL,
    .cbPduQueued = NULL,
    .cbPduTransmitted = Ble_LlcpConnectionUpdatePduTransmitted,
    .cbFinished = Ble_LlcpConnectionUpdateFinished,
};

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_ConnectionParamReqPduDescriptors[] =
{
    {gpBleLlcp_OpcodeConnectionParamReq, 23, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH},
    {gpBleLlcp_OpcodeConnectionParamRsp, 23, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_PERIPHERAL},
    BLE_INIT_CONNECTION_UPDATE_PDU_DESCRIPTOR(),
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_ConnectionParamReqDescriptor =
{
    .procedureFlags = GPBLELLCPFRAMEWORK_PROCEDURE_FLAGS_INSTANT_BM,
    .procedureDataLength = sizeof(Ble_LlcpConnParamReqRspData_t),
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_ConnectionParamReqPduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_ConnectionParamReqPduDescriptors,
    .featureMask = BM(gpBleConfig_FeatureIdConnectionParametersRequest),
    .cbQueueingNeeded = NULL,
    .cbProcedureStart = NULL,
    .cbGetCtrData = Ble_LlcpConnectionParamReqGetCtrData,
    .cbStoreCtrData = Ble_LlcpConnectionParamReqStoreCtrData,
    .cbPduReceived = Ble_LlcpConnectionParamReqPduReceived,
    .cbUnexpectedPduReceived = Ble_LlcpConnectionParamReqUnexpectedPduReceived,
    .cbPduQueued = Ble_LlcpConnectionParamReqPduQueued,
    .cbPduTransmitted = Ble_LlcpConnectionParamReqPduTransmitted,
    .cbFinished = Ble_LlcpConnectionParamReqFinished,
};
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

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

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
gpHci_Result_t Ble_ConnectionParamReqReplyAction(Ble_IntConnId_t connId, gpHci_LeRemoteConnectionParamRequestReplyCommand_t* pRequest)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;
    Ble_LlcpConnParamReqRspData_t* pConnReqPdu;
    gpBleLlcp_Opcode_t startPdu;

    // Use the data we got from the host to send the response
    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pRequest != NULL);

    pProcedure = gpBleLlcpFramework_GetProcedure(pContext, false);

    if(!BleLlcpProcedures_ConnParamReqReplyCmdAllowed(pProcedure))
    {
        return gpHci_ResultCommandDisallowed;
    }

    pConnReqPdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    GP_ASSERT_DEV_INT(pConnReqPdu != NULL);

    pConnReqPdu->pdu.intervalMin = pRequest->connIntervalMin;
    pConnReqPdu->pdu.intervalMax = pRequest->connIntervalMax;
    pConnReqPdu->pdu.latency= pRequest->connLatency;
    pConnReqPdu->pdu.timeout = pRequest->supervisionTimeout;
    pConnReqPdu->minCELength = pRequest->minCELength;
    pConnReqPdu->maxCELength = pRequest->maxCELength;

    if(pContext->masterConnection)
    {
        startPdu = gpBleLlcp_OpcodeConnectionUpdateInd;
    }
    else
    {
        startPdu = gpBleLlcp_OpcodeConnectionParamRsp;
    }

    gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_HOST_IDX);

    gpBleLlcpFramework_ResumeProcedure(pContext, pProcedure, startPdu);

    return gpHci_ResultSuccess;
}

gpHci_Result_t Ble_ConnectionParamReqNegReplyAction(Ble_IntConnId_t connId, gpHci_Result_t reason)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    pProcedure = gpBleLlcpFramework_GetProcedure(pContext, false);

    if(!BleLlcpProcedures_ConnParamReqReplyCmdAllowed(pProcedure))
    {
        return gpHci_ResultCommandDisallowed;
    }

    pProcedure->result = reason;

    gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX);
    gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_HOST_IDX);

    gpBleLlcpFramework_ResumeProcedure(pContext, pProcedure, gpBleLlcp_OpcodeRejectExtInd);

    return gpHci_ResultSuccess;
}
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

//-----------------------------
// Connection Update procedure
//-----------------------------


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

    if(!RANGE_CHECK(hopIncrement, BLE_INITIATOR_HOP_FIELD_MIN, BLE_INITIATOR_HOP_FIELD_MAX) || (unmappedChannelPtr >= BLE_DATA_NUMBER_OF_CHANNELS))
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

void Ble_LlcpConnectionUpdatePreInstantPassed(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
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

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

//----------------------------------------
// Connection Parameters Request procedure
//----------------------------------------

void Ble_LlcpConnectionParamReqGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData)
{
    UInt8 index = 0;
    Ble_LlcpConnParamReqRspData_t* pData = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    switch(*pOpcode)
    {
        case gpBleLlcp_OpcodeConnectionParamReq:
        {
            pData->pdu.preferredPeriodicity = BleLLcpProcedures_GetPreferredPeriodicity(pData->pdu.intervalMax);
            Ble_LlcpPrepareConnectionParamReqRspData(pContext, pData, pCtrDataLength, pCtrData);

            break;
        }
        case gpBleLlcp_OpcodeConnectionParamRsp:
        {
            Ble_LlcpPrepareConnectionParamReqRspData(pContext, pData, pCtrDataLength, pCtrData);

            break;
        }
        case gpBleLlcp_OpcodeRejectExtInd:
        {
            // || RejectOpcode (1) | Error code (1) ||
            pCtrData[index++] = pProcedure->expectedRxPdu;
            pCtrData[index++] = pProcedure->result;

            *pCtrDataLength = index;

            gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX);
            break;
        }
        default:
        {
            // Implement
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }
}

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(opcode)
    {
        case gpBleLlcp_OpcodeConnectionParamReq:
        {
            Ble_LlcpConnParamReqRspData_t* pConnReqPdu;

            pConnReqPdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

            Ble_LlcpReadConnParamReqRspPdu(&pConnReqPdu->pdu, pPdLoh);

            if(!Ble_ConnectionParametersValid(pConnReqPdu->pdu.intervalMin, pConnReqPdu->pdu.intervalMax, pConnReqPdu->pdu.latency, pConnReqPdu->pdu.timeout, 0,0))
            {
                pProcedure->result = gpHci_ResultInvalidLMPParametersInvalidLLParameters;
                return Ble_LlcpFrameworkActionReject;
            }


            break;
        }
        case gpBleLlcp_OpcodeConnectionParamRsp:
        {
            Ble_LlcpConnParamReqRspPdu_t rspPdu;
            Ble_LlcpConnParamReqRspData_t* pStoredConnReq;

            pStoredConnReq = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

            Ble_LlcpReadConnParamReqRspPdu(&rspPdu, pPdLoh);

            if(!Ble_ConnectionParametersValid(rspPdu.intervalMin, rspPdu.intervalMax, rspPdu.latency, rspPdu.timeout, 0,0))
            {
                pProcedure->result = gpHci_ResultInvalidLMPParametersInvalidLLParameters;
                return Ble_LlcpFrameworkActionReject;
            }

            // Overwrite own values: behave friendly to the remote and use his values
            MEMCPY(&pStoredConnReq->pdu, &rspPdu, sizeof(Ble_LlcpConnParamReqRspPdu_t));

            break;
        }
        case gpBleLlcp_OpcodeConnectionUpdateInd:
        {
            action = Ble_LlcpConnectionUpdateReqStoreCtrData(pContext, pProcedure, pPdLoh, opcode);
            break;
        }
        default:
        {
            // Implement
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;


    GP_ASSERT_DEV_INT(pProcedure);

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodeConnectionParamReq:
        {
            if(pContext->masterConnection)
            {
                pProcedure->expectedRxPdu = gpBleLlcp_OpcodeConnectionParamRsp;
            }
            else
            {


                pProcedure->expectedRxPdu = gpBleLlcp_OpcodeConnectionUpdateInd;
            }
            break;
        }
        case gpBleLlcp_OpcodeConnectionParamRsp:
        {


            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeConnectionUpdateInd;
            break;
        }
        case gpBleLlcp_OpcodeConnectionUpdateInd:
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;
            break;
        }
        default:
        {
            // Should not happen
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }


    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqPduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    // We need to suspend execution untill the configured instant occurs

    if(txOpcode == gpBleLlcp_OpcodeConnectionParamRsp || txOpcode == gpBleLlcp_OpcodeConnectionUpdateInd)
    {
        return Ble_LlcpFrameworkActionPause;
    }
    else if(txOpcode == gpBleLlcp_OpcodeRejectExtInd)
    {
        return Ble_LlcpFrameworkActionStop;
    }

    return Ble_LlcpFrameworkActionContinue;
}

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    Ble_LlcpConnParamReqRspData_t* pConnUpdatePdu;

    pConnUpdatePdu = (Ble_LlcpConnParamReqRspData_t*)pProcedure->pData;

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodeConnectionParamReq:
        {
            GP_ASSERT_DEV_INT(!pProcedure->localInit);

            if(pContext->masterConnection)
            {
                pProcedure->currentTxPdu = gpBleLlcp_OpcodeConnectionUpdateInd;
            }
            else
            {
                pProcedure->currentTxPdu = gpBleLlcp_OpcodeConnectionParamRsp;
            }

            if((pConnUpdatePdu->pdu.intervalMin == pConnUpdatePdu->pdu.intervalMax) && Ble_LlcpAnchorMoveRequested(pContext->connId, pConnUpdatePdu->pdu.intervalMin, pConnUpdatePdu->pdu.latency, pConnUpdatePdu->pdu.timeout))
            {
                // Do not indicate requests that are anchor moves

                // Continue using the current minCE and maxCE values
                pConnUpdatePdu->minCELength = pContext->ccParams.minCELength;
                pConnUpdatePdu->maxCELength = pContext->ccParams.maxCELength;
            }
            else if(Ble_LlcpConnParamChangeRequested(pContext->connId, pConnUpdatePdu->pdu.intervalMin, pConnUpdatePdu->pdu.intervalMax, pConnUpdatePdu->pdu.latency, pConnUpdatePdu->pdu.timeout) || !pContext->masterConnection)
            {
                // Only trigger this when we are slave AND the host has not yet indicated the parameters to the link layer
                gpHci_EventCbPayload_t payload;

                // We need interaction with the host in case any of the connection parameters changed
                GP_LOG_PRINTF("Pause conn param req, send event",0);

                payload.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeRemoteConnectionParameter;
                payload.metaEventParams.params.remoteConnectionParams.connectionHandle = pContext->hciHandle;
                payload.metaEventParams.params.remoteConnectionParams.connIntervalMin = pConnUpdatePdu->pdu.intervalMin;
                payload.metaEventParams.params.remoteConnectionParams.connIntervalMax = pConnUpdatePdu->pdu.intervalMax;
                payload.metaEventParams.params.remoteConnectionParams.connLatency = pConnUpdatePdu->pdu.latency;
                payload.metaEventParams.params.remoteConnectionParams.supervisionTimeout = pConnUpdatePdu->pdu.timeout;

                gpBle_ScheduleEvent(0, gpHci_EventCode_LEMeta, &payload);

                gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_HOST_IDX);

                return Ble_LlcpFrameworkActionPause;
            }

            break;
        }
        case gpBleLlcp_OpcodeConnectionParamRsp:
        {
            pProcedure->currentTxPdu = gpBleLlcp_OpcodeConnectionUpdateInd;
            break;
        }
        case gpBleLlcp_OpcodeConnectionUpdateInd:
        {
            Ble_LlcpConnectionUpdatePduReceived(pContext, pProcedure, rxOpcode);


            break;
        }
        default:
        {
            // We should not end up here (handling done in Ble_LlcpConnectionParamReqUnexpectedPduReceived)
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return Ble_LlcpFrameworkActionContinue;
}

Ble_LlcpFrameworkAction_t Ble_LlcpConnectionParamReqUnexpectedPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh)
{
    if(pProcedure->currentTxPdu == gpBleLlcp_OpcodeConnectionUpdateInd)
    {
        // PDU can be ignored, since we do not expect a response at this point
        return Ble_LlcpFrameworkActionContinue;
    }
    else
    {
        return Ble_LlcpCommonUnexpectedPduReceived(pContext, pProcedure, rxOpcode, pPdLoh);
    }
}

void Ble_LlcpConnectionParamReqFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    Ble_LlcpConnectionUpdateFinishedCommon(pContext, pProcedure, notifyHost);
}
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED


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

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
void Ble_LlcpRegisterConnectionParamReqProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdConnectionParamRequest, &BleLlcpProcedures_ConnectionParamReqDescriptor);
}
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

//--------
// Various
//--------

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
void Ble_LlcpReadConnParamReqRspPdu(Ble_LlcpConnParamReqRspPdu_t* pPdu, gpPd_Loh_t* pPdLoh)
{
    MEMSET(pPdu, 0, sizeof(Ble_LlcpConnParamReqRspPdu_t));

    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pPdu->intervalMin);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pPdu->intervalMax);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pPdu->latency);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pPdu->timeout);
    gpPd_ReadWithUpdate(pPdLoh, 1, (UInt8*)&pPdu->preferredPeriodicity);
    gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&pPdu->refConnEventCount);
    // Offset0 - Offset5
    gpPd_ReadWithUpdate(pPdLoh, 2 * BLE_CONN_PARAM_REQ_NR_OF_OFFSETS, (UInt8*)&pPdu->offsets);
}

void Ble_LLcpConnParamReqPduToBuffer(Ble_LlcpConnParamReqRspData_t* pPdu, UInt8* pCtrData, UInt8* pCtrDataLength)
{
    UInt8 index = 0;

    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pPdu->pdu.intervalMin, &index, 2);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pPdu->pdu.intervalMax, &index, 2);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pPdu->pdu.latency, &index, 2);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pPdu->pdu.timeout, &index, 2);
    gpBle_AppendWithUpdate(&pCtrData[index], &pPdu->pdu.preferredPeriodicity, &index, 1);
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)&pPdu->pdu.refConnEventCount, &index, 2);
    // Offset0 - Offset5
    gpBle_AppendWithUpdate(&pCtrData[index], (UInt8*)pPdu->pdu.offsets, &index, 2*BLE_CONN_PARAM_REQ_NR_OF_OFFSETS);

    *pCtrDataLength = index;
}

void Ble_LlcpPrepareConnectionParamReqRspData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpConnParamReqRspData_t* pConnReqPdu, UInt8* pCtrDataLength, UInt8* pCtrData)
{
    // Invalidate all offsets, will be updated later on when needed
    MEMSET(pConnReqPdu->pdu.offsets, 0xFF, sizeof(pConnReqPdu->pdu.offsets));

    // Offset calculation only meaningful for masters when interval is known
    if(pContext->masterConnection && (pConnReqPdu->pdu.intervalMin == pConnReqPdu->pdu.intervalMax))
    {
        UInt16 offset;
        UInt16 offsetToReference;
        gpBleActivityManager_Timing_t anchorPoint;
        gpHal_ConnEventMetrics_t connMetrics;
        gpBleActivityManager_ConnUpdateInput_t inputParams;

        // First fetch some metrics from this connection
        gpHal_BleGetConnectionMetrics(pContext->connId, &connMetrics);

        inputParams.currentEventTs = connMetrics.nextAnchorTime;
        inputParams.intervalMin = pConnReqPdu->pdu.intervalMin;
        inputParams.intervalMax = pConnReqPdu->pdu.intervalMax;
        inputParams.currentInterval = pContext->intervalUnit;
        inputParams.currentLatency = pContext->latency;
        inputParams.preferredPeriodicity = pConnReqPdu->pdu.preferredPeriodicity;

        // Now, request suitable anchor points from the activity manager
        gpBleActivityManager_GetPreferredAnchorPoint(pContext->connId, &inputParams, &anchorPoint);

        offset = (anchorPoint.firstActivityTs - connMetrics.nextAnchorTime) % pConnReqPdu->pdu.intervalMin;
        offsetToReference = (anchorPoint.firstActivityTs - offset - connMetrics.nextAnchorTime) / pContext->intervalUnit;
        pConnReqPdu->pdu.refConnEventCount = connMetrics.eventCounterNext + offsetToReference;
        pConnReqPdu->pdu.offsets[0] = offset;
    }

    // Invalidate preferredPeriodicty before continueing (SDP004-894)
    pConnReqPdu->pdu.preferredPeriodicity = BleLLcpProcedures_GetPreferredPeriodicity(pConnReqPdu->pdu.intervalMax);

    Ble_LLcpConnParamReqPduToBuffer(pConnReqPdu, pCtrData, pCtrDataLength);
}
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

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

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
    // connection update maps to connection parameters request procedure if supported on both sides
    startDescriptor.procedureId = gpBleLlcp_ProcedureIdConnectionParamRequest;
    result = gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

    if(pContext->masterConnection && (result == gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature))
    {
        startDescriptor.procedureId = gpBleLlcp_ProcedureIdConnectionUpdate;
        // On a master, use connection update procedure if one (or both) sides do not support the connections parameter request procedure
        result = gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
    }

    return result;
}

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
Bool BleLlcpProcedures_ConnParamReqReplyCmdAllowed(Ble_LlcpProcedureContext_t* pProcedure)
{
    if(pProcedure == NULL)
    {
        GP_LOG_PRINTF("Reply cmd: no procedure",0);
        return false;
    }

    if(pProcedure->procedureId != gpBleLlcp_ProcedureIdConnectionParamRequest)
    {
        GP_LOG_PRINTF("Reply cmd: wrong procedure",0);
        return false;
    }

    if(!gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_HOST_IDX))
    {
        GP_LOG_PRINTF("Reply cmd: wrong state",0);
        return false;
    }

    return true;
}
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED


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
#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
    Ble_LlcpRegisterConnectionParamReqProcedure();
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
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

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
gpHci_Result_t gpBle_LeRemoteConnectionParamRequestReply(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_LeRemoteConnectionParamRequestReplyCommand_t* pRequest = &pParams->LeRemoteConnectionParamRequestReply;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    pEventBuf->payload.commandCompleteParams.returnParams.connectionHandle = pRequest->connectionHandle;

    result = Ble_ConnectionParametersChecker(
        pRequest->connectionHandle, pRequest->connIntervalMin, pRequest->connIntervalMax,
        pRequest->connLatency, pRequest->supervisionTimeout, pRequest->minCELength, pRequest->maxCELength
    );

    GP_LOG_SYSTEM_PRINTF("REM conn param req rep res %u",0, result);

    if(result == gpHci_ResultSuccess)
    {
        result = Ble_ConnectionParamReqReplyAction(gpBleLlcp_HciHandleToIntHandle(pRequest->connectionHandle), pRequest);
    }

    return result;
}

gpHci_Result_t gpBle_LeRemoteConnectionParamRequestNegReply(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_LeRemoteConnectionParamRequestNegReplyCommand_t* pRequest = &pParams->LeRemoteConnectionParamRequestNegReply;

    if(pEventBuf != NULL)
    {
        // This check is needed, because this function can be called directly when the event is masked
        BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
        pEventBuf->payload.commandCompleteParams.returnParams.connectionHandle = pRequest->connectionHandle;
    }

    result = gpBleLlcp_IsHostConnectionHandleValid(pRequest->connectionHandle);

    if(result == gpHci_ResultSuccess)
    {
        result = Ble_ConnectionParamReqNegReplyAction(gpBleLlcp_HciHandleToIntHandle(pRequest->connectionHandle), pRequest->reason);
    }

    return result;
}
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED

