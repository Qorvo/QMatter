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

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLELLCP

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
// #include "gpBle_defs.h"
#include "gpBleConfig.h"
#include "gpBleComps.h"
#include "gpBleDataChannelRxQueue.h"
#include "gpBleDataChannelTxQueue.h"
#include "gpBleDataCommon.h"
#include "gpBleDataRx.h"
#include "gpBleDataTx.h"
#include "gpBleLlcp.h"
#include "gpBle_LLCP_getters.h"
#include "gpBleLlcpFramework.h"
#include "gpBleLlcpProcedures.h"
#include "gpBle_defs.h"
#include "gpPoolMem.h"
#include "gpSched.h"
#include "gpLog.h"
#include "gpPd.h"
#include "gpHal.h"
#include "gpHal_Ble_Manual.h"
#include "gpBleAddressResolver.h"
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#include "gpBle_LLCP_getters.h"
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#ifdef GP_COMP_BLERESPRADDR
#include "gpBleResPrAddr.h"
#endif
#include "inttypes.h"
#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
#include "gpBleLlcpProcedures_ConstantToneExtension.h"
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_CONN_REQ_PAYLOAD_LENGTH         (6 + 6 + 22)

#define BLE_VIRT_CONN_PRIORITY                      Ble_Priority_High
#ifdef GP_DIVERSITY_AGGRESSIVE_WIN_WIDENING_THRESHOLD
#define BLE_FIXED_WD_THRESHOLD_DEFAULT              GP_DIVERSITY_AGGRESSIVE_WIN_WIDENING_THRESHOLD
#else
// Note that aggressive window widening does not work nicely with many simultaneous slave links, especially with small values of BLE_FIXED_WD_THRESHOLD_DEFAULT
#define BLE_FIXED_WD_THRESHOLD_DEFAULT              3
#endif // GP_DIVERSITY_AGGRESSIVE_WIN_WIDENING_THRESHOLD

#define BLE_CONNECTION_LEGACY_TRANSMIT_BLACKOUT_US  1250

#define BLE_MAX_SLEEP_TIME_WITH_RC_MODE_US                  MS_TO_US(1000)

#ifndef GP_DIVERSITY_BLELLCP_RPA_CACHE_TIMEOUT_MS
#define GP_DIVERSITY_BLELLCP_RPA_CACHE_TIMEOUT_MS     3000
#endif //GP_DIVERSITY_BLELLCP_RPA_CACHE_TIMEOUT_MS

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_GET_COMBINED_SCA_COMPENSATION(master,slave)     (master + slave)*105/100

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    UInt8 nrOfSlaveConnections;
    UInt8 nrOfMasterConnections;
    gpHal_BleChannelMapHandle_t masterChannelMapHandle;
} Ble_LlcpGlobalContext;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static Ble_LlcpLinkContext_t Ble_LinkInfo[BLE_LLCP_MAX_NR_OF_CONNECTIONS];

static Ble_LlcpGlobalContext Ble_GlobalContext;

#ifdef GP_DIVERSITY_DEVELOPMENT
static Bool gpBle_VsdAutomaticFeatureExchangeEnabled;
static Int16 gpBle_VsdMasterConnEstabFirstMToSSignedOffset;
static Int16 gpBle_VsdArtificialDriftAsSignedNbrMicrosec;
static Bool gpBle_VsdExternalAccessCodeValid;
static UInt32 gpBle_VsdExternalAccessCode;
#endif /* #ifdef GP_DIVERSITY_DEVELOPMENT */

static UInt16 gpBle_VsdfixedRxWindowThreshold;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

// checker/action functions
#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
static gpHci_Result_t Ble_WriteAuthenticatedPayloadTOChecker(gpHci_WriteAuthenticatedPayloadTOCommand_t* pWriteParams);
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED


static void Ble_LlcpPopulateConnEventInfo(Ble_LlcpLinkContext_t* pContext, gpHal_ConnEventInfo_t* pConnEventInfo, Ble_ConnEstablishParams_t* pConnEstablishParams, UInt8 chanMapHandle,
    UInt32 firstTheoreticalConnEventTs, UInt16 masterSca, UInt8 nrOfSkippedIntervals, UInt32 InitialWindowSize);

static gpHci_Result_t Ble_LlcpStartConnectionEstablishmentMaster(Ble_IntConnId_t connId, Ble_ConnEstablishParams_t* pConnEstablishParams);
static void Ble_LlcpStartConnectionEstablishmentSlave(Ble_IntConnId_t connHandle, Ble_ConnEstablishParams_t* pConnEstablishParams);
static void Ble_LlcpStartConnectionEstablishmentCommon(Ble_LlcpLinkContext_t* pContext, Ble_ConnEstablishParams_t* pConnEstablishParams, UInt8 nrOfSkippedIntervals, UInt32 firstTheoreticalConnEventTs);

#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
static void Ble_PrepareConnectionComplete(gpHci_LEEnhancedConnectionCompleteEventParams_t* pConnCompleteParams, gpHci_ConnectionHandle_t connHandle, gpHci_ConnectionRole_t role, Ble_ConnEstablishParams_t* pConnEstablishParams);
#else
static void Ble_PrepareConnectionComplete(gpHci_LEConnectionCompleteEventParams_t* pConnCompleteParams, gpHci_ConnectionHandle_t connHandle, gpHci_ConnectionRole_t role, Ble_ConnEstablishParams_t* pConnEstablishParams);
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE

static void Ble_LlcpResetConnection(Ble_LlcpLinkContext_t* pContext, Bool permanentConnection);
static void Ble_StopConnectionFollowUp(void* pArgs);

// Link context management
static Ble_LlcpLinkContext_t* Ble_AllocateLinkContext(void);
static void Ble_FreeLinkContext(Ble_IntConnId_t connId);

// various
Bool Ble_IsConnectionIntervalValid(UInt16 connIntervalMin, UInt16 connIntervalMax);
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
static void Ble_TriggerCbConnectionComplete(Ble_LlcpLinkContext_t* pContext, gpHci_LEEnhancedConnectionCompleteEventParams_t* pParams);
#else
static void Ble_TriggerCbConnectionComplete(Ble_LlcpLinkContext_t* pContext, gpHci_LEConnectionCompleteEventParams_t* pParams);
#endif //GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE



static UInt32 BleLlcp_GetEarliestAnchorPoint(Ble_ConnEstablishParams_t* pConnEstablishParams);
static UInt32 BleLlcp_GetSlaveFirstRxTimestamp(Ble_LlcpLinkContext_t* pContext, UInt32 earliestAnchorPoint, Ble_ConnEstablishParams_t* pConnEstablishParams, UInt32* pWindowDuration);
static UInt32 BleLlcp_CompensateAfterBlackout(Ble_LlcpLinkContext_t* pContext, UInt32 earliestAnchorPoint, UInt32 blackoutUs, UInt32 winOffset, UInt32 winSizeUs, UInt32* pWindowDuration);
static void BleLlcp_StartInitialProcedures(void* pArg);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
gpHci_Result_t Ble_WriteAuthenticatedPayloadTOChecker(gpHci_WriteAuthenticatedPayloadTOCommand_t* pWriteParams)
{
    Ble_LlcpLinkContext_t* pContext;

    if(gpBleLlcp_IsHostConnectionHandleValid(pWriteParams->connectionHandle) != gpHci_ResultSuccess)
    {
        GP_LOG_PRINTF("Invalid connection id: %x",0, pWriteParams->connectionHandle);
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    pContext = Ble_GetLinkContext(gpBleLlcp_HciHandleToIntHandle(pWriteParams->connectionHandle));

    // Should not be invalid, a check on the connectionHandle has been done before
    GP_ASSERT_DEV_INT(pContext != NULL);

    if(pWriteParams->authenticatedPayloadTO == 0)
    {
        GP_LOG_PRINTF("Auth payload TO cannot be 0",0);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    if(BLE_TIME_UNIT_10000_TO_US(pWriteParams->authenticatedPayloadTO) < BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit) * (1 + pContext->latency))
    {
        GP_LOG_PRINTF("Auth payload TO (%x) should be >= %x",0,pWriteParams->authenticatedPayloadTO, (pContext->intervalUnit*(1 + pContext->latency)));
        return gpHci_ResultInvalidHCICommandParameters;
    }
    return gpHci_ResultSuccess;
}
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED


// Link context allocation and freeing
Ble_LlcpLinkContext_t* Ble_AllocateLinkContext(void)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LinkInfo[i].connId == BLE_CONN_HANDLE_INVALID)
        {
            UIntLoop j;

            Ble_LinkInfo[i].connectionCompleteBufferHandle = gpBle_EventBufferToHandle(gpBle_AllocateEventBuffer(Ble_EventBufferType_ConnectionComplete));

            if(Ble_LinkInfo[i].connectionCompleteBufferHandle == GP_BLE_EVENT_HANDLE_IDX_INVALID)
            {
                GP_LOG_SYSTEM_PRINTF("WARN: Could not claim buffer for conn complete.",0);
                break;
            }

            Ble_LinkInfo[i].connId = i;
            Ble_LinkInfo[i].hciHandle = GP_HCI_CONNECTION_HANDLE_INVALID;
            Ble_LinkInfo[i].pProcedureLocal = NULL;
            Ble_LinkInfo[i].pProcedureRemote = NULL;
            Ble_LinkInfo[i].versionExchanged = false;
            Ble_LinkInfo[i].featuresExchangedStatus = gpBleLlcp_FeatureStatus_NotExchanged;
            Ble_LinkInfo[i].terminationOngoing = false;
            Ble_LinkInfo[i].localProcedureFlowEnabled = true;
            Ble_LinkInfo[i].remoteProcedureFlowEnabled = true;
            Ble_LinkInfo[i].encryptionEnabled = false;
            Ble_LinkInfo[i].accessAddress = 0x0;
            Ble_LinkInfo[i].peerAddressType = gpHci_InitPeerAddressType_Invalid;
            Ble_LinkInfo[i].rxLlcpOpcode = gpBleLlcp_OpcodeInvalid;
            Ble_LinkInfo[i].queuedPdLoh.handle = GP_PD_INVALID_HANDLE;

            Ble_LinkInfo[i].nrOfProceduresUsingPrst = 0;

            // Default for combined featureSet (local and remote) and allowedProcedures is every bit set.
            // If feature exchange was not successful, this would still allow the LL to trigger a procedure
            // If the remote responds with an LL_UNKNOWN_RSP, the bit will be cleared.
            MEMSET(&Ble_LinkInfo[i].featureSetLink, 0xFF, GP_HCI_FEATURE_SET_SIZE);
            MEMSET(&Ble_LinkInfo[i].allowedProcedures, 0xFF, GP_HCI_FEATURE_SET_SIZE);
            MEMSET(&Ble_LinkInfo[i].peerAddress, 0, sizeof(Ble_LinkInfo[i].peerAddress));
            MEMSET(&Ble_LinkInfo[i].remoteVersionInfo, 0, sizeof(Ble_LinkInfo[i].remoteVersionInfo));
            for(j = 0; j < BLE_LLCP_MAX_NR_OF_BUFFERED_RESOURCES_PER_LINK; j++)
            {
                Ble_LinkInfo[i].pdus[j].length = 0;
                Ble_LinkInfo[i].pdus[j].origin = Ble_LlcpPduResourceOriginInvalid;
            }

            return &Ble_LinkInfo[i];
        }
    }

    return NULL;
}

Ble_LlcpLinkContext_t* Ble_GetLinkContext(Ble_IntConnId_t connId)
{
    if(!BLE_IS_INT_CONN_HANDLE_VALID(connId))
    {
        return NULL;
    }
    if(BLE_IS_INT_CONN_HANDLE_VALID(Ble_LinkInfo[connId].connId))
    {
        return &Ble_LinkInfo[connId & ~(GPHAL_BLE_VIRTUAL_CONN_MASK)];
    }
    return NULL;
}

void Ble_FreeLinkContext(Ble_IntConnId_t connId)
{
    MEMSET(&Ble_LinkInfo[connId].peerAddress, 0, sizeof(BtDeviceAddress_t));

    if(Ble_LinkInfo[connId].connectionCompleteBufferHandle != GP_BLE_EVENT_HANDLE_IDX_INVALID)
    {
        gpBle_HciEventConfirm(Ble_LinkInfo[connId].connectionCompleteBufferHandle);
        Ble_LinkInfo[connId].connectionCompleteBufferHandle = GP_BLE_EVENT_HANDLE_IDX_INVALID;
    }

    Ble_LinkInfo[connId].connId = BLE_CONN_HANDLE_INVALID;
    Ble_LinkInfo[connId].hciHandle = GP_HCI_CONNECTION_HANDLE_INVALID;
    Ble_LinkInfo[connId].connectionStatus = gpBleLlcp_ConnectionStatus_NotConnected;
}

Bool Ble_LlcpIsConnected(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    return Ble_LinkInfo[connId].connectionStatus == gpBleLlcp_ConnectionStatus_Connected;
}

Bool Ble_IsConnectionActive(BtDeviceAddress_t* pAddress)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LinkInfo[i].connId != BLE_CONN_HANDLE_INVALID)
        {
            if( (MEMCMP(pAddress->addr, Ble_LinkInfo[i].peerAddress.addr, sizeof(BtDeviceAddress_t)) == 0) &&
                (Ble_LlcpIsConnected(i)) )
            {
                return true;
            }
        }
    }
    return false;
}

Bool Ble_IsAccessAddressUsed(gpBle_AccessAddress_t accessAddress)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LinkInfo[i].connId != BLE_CONN_HANDLE_INVALID)
        {
            if(Ble_LinkInfo[i].accessAddress == accessAddress)
            {
                return true;
            }
        }
    }
    return false;
}

gpBleLlcp_FeatureStatus_t gpBleLlcp_IsFeatureSupported(gpHci_ConnectionHandle_t connectionHandle, gpBleConfig_FeatureId_t featureId)
{
    Ble_LlcpLinkContext_t* pContext = Ble_GetLinkContext(gpBleLlcp_HciHandleToIntHandle(connectionHandle));

    if(pContext == NULL || !GPBLELLCP_IS_FEATURE_VALID(featureId))
    {
        return gpBleLlcp_FeatureStatus_NotSupported;
    }

    if(featureId == gpBleConfig_FeatureIdLePing)
    {
        // The ping procedure is special. The only purpose of the ping is checking if the remote is still there.
        // Even if the remote does not support this feature, it will answer with an encrypted PDU.
        return gpBleLlcp_FeatureStatus_Supported;
    }

    if(pContext->featuresExchangedStatus == gpBleLlcp_FeatureStatus_ExchangedSuccess)
    {
        // The features were succesfully exchanged, the bits in featureSetLink indicate the support status
        if(BIT_TST(pContext->featureSetLink, featureId))
        {
            return gpBleLlcp_FeatureStatus_Supported;
        }
        else
        {
            return gpBleLlcp_FeatureStatus_NotSupported;
        }
    }
    else
    {
        // Features were not yet (succesfully) exchanged, which means we cannot rely on featureSetLink
        // Check allowedProcedures to make sure we don't trigger an unsupported feature again
        if(BIT_TST(pContext->allowedProcedures, featureId) == 0)
        {
            // The bit is cleared ==> Not supported
            return gpBleLlcp_FeatureStatus_NotSupported;
        }
        else
        {
            // We cannot be sure, return MaybeSupported
            return gpBleLlcp_FeatureStatus_MaybeSupported;
        }
    }
}

void Ble_GetAccessAddress(Ble_IntConnId_t connId, gpBle_AccessAddress_t* pAccessAddress)
{
    GP_ASSERT_DEV_INT(Ble_LinkInfo[connId].connId != BLE_CONN_HANDLE_INVALID);
    MEMCPY(pAccessAddress, &Ble_LinkInfo[connId].accessAddress, sizeof(gpBle_AccessAddress_t));
}

void Ble_LlcpGetPeerAddressInfo(Ble_IntConnId_t connId, Bool* pAddressType, BtDeviceAddress_t* pAddress)
{
    Ble_LlcpLinkContext_t* pContext;

    GP_ASSERT_DEV_INT(Ble_LinkInfo[connId].connId != BLE_CONN_HANDLE_INVALID);

    pContext = Ble_GetLinkContext(connId);

    if(pContext != NULL)
    {
        *pAddressType = pContext->peerAddressType?1:0;
        MEMCPY(pAddress, &pContext->peerAddress, sizeof(BtDeviceAddress_t));
    }
}

UInt32 Ble_DriftCompensation(UInt16 combinedScaPpm, UInt32 elapsedTimeUs)
{
    UInt32 result;
    UInt32 elapsedTimeMs;

    elapsedTimeMs = (elapsedTimeUs/1000) + 1;
    result = combinedScaPpm*elapsedTimeMs;

    result /= 1000;

    return result;
}

void Ble_LlcpPopulateConnEventInfo(Ble_LlcpLinkContext_t* pContext, gpHal_ConnEventInfo_t* pConnEventInfo, Ble_ConnEstablishParams_t* pConnEstablishParams, UInt8 chanMapHandle,
    UInt32 firstTheoreticalConnEventTs, UInt16 masterSca, UInt8 nrOfSkippedIntervals, UInt32 InitialWindowSize)
{
    UInt16 connMask;
    Ble_ConnReqPdu_t* pConnReqPdu;

    MEMSET(pConnEventInfo, 0, sizeof(gpHal_ConnEventInfo_t));

    pConnReqPdu = &pConnEstablishParams->connReqPdu;

    pConnEventInfo->rtEvent.priority = BLE_CONN_PRIORITY;

    // Always enable extended priority mechanism (round robin that takes number of skipped events into account)
    pConnEventInfo->rtEvent.enableExtPriority = true;
    pConnEventInfo->rtEvent.intervalUs = BLE_TIME_UNIT_1250_TO_US(pConnReqPdu->llData.interval);

#ifdef GP_DIVERSITY_DEVELOPMENT
    pConnEventInfo->rtEvent.intervalUs += gpBle_VsdArtificialDriftAsSignedNbrMicrosec;
#endif /* GP_DIVERSITY_DEVELOPMENT */

    pConnEventInfo->channelId = 0x0;
    pConnEventInfo->hopIncrement= pConnReqPdu->llData.hopIncrement;
    pConnEventInfo->unmappedChannelPtr = 0x0;
    // Note : hop increment has a max value of 16, and nrOfSkippedIntervals < 6 (or we'll loose the link anyway)
    // So, hopIncrement * nrOfSkippedIntervals < 255 : no need to worry about UInt8 overflow here
    pConnEventInfo->unmappedChannelPtr = (pConnEventInfo->hopIncrement * nrOfSkippedIntervals) % BLE_DATA_NUMBER_OF_CHANNELS;
    pConnEventInfo->accessAddress= pConnReqPdu->llData.accessAddress;
    pConnEventInfo->crcInit = pConnReqPdu->llData.crcInit;
    // windowDuration is a slave-only property
    pConnEventInfo->windowDuration = InitialWindowSize;
    pConnEventInfo->fixedWDThreshold = gpBle_VsdfixedRxWindowThreshold;
    pConnEventInfo->masterSca = masterSca;          // Only needed for slave to have a value of masterSCA (since he needs to listen to master first), 0 for master
    pConnEventInfo->slaveLatency = 0;               // slave latency must be disabled until the master has sent the first ack - this is taken care of by the connections monitor
    pConnEventInfo->eventCounter = nrOfSkippedIntervals;
    pConnEventInfo->channelMapHandle = chanMapHandle;
    pConnEventInfo->txSeqNr = 0;
    pConnEventInfo->txNextSeqNr = 0;
    // Initialize to one, used to forward the first packet of a connection
    pConnEventInfo->rxSeqNr = 1;
    pConnEventInfo->rxNextSeqNr = 0;
    pConnEventInfo->lastConnEventCountValid = 0x0;
    pConnEventInfo->lastConnEventCount = 0x0;
    // Check if flow is enabled for this connection
    connMask = gpBle_DataRxQueueGetFlowCtrl();
    pConnEventInfo->rxFlowCtrlFlag = ((UInt16)(connMask>>pContext->connId)&0x01);
    // Initialize last correlatio time to the Ts of the Connect_Req, i.e. before the first (theoretical) connection event
    pConnEventInfo->tsLastValidPacketReceived = firstTheoreticalConnEventTs;
    pConnEventInfo->tsLastPacketReceived = firstTheoreticalConnEventTs;

    pConnEventInfo->preamble = gpBle_GetPreambleSymbol(gpBleDataCommon_PhyWithCodingToPhy(pConnEstablishParams->phy), pConnReqPdu->llData.accessAddress);

    pConnEventInfo->rtEvent.nrOfConsecSkippedEvents = nrOfSkippedIntervals;


    pConnEventInfo->phy = gpBleDataCommon_HciPhyWithCodingToHalPhyWithCoding(pConnEstablishParams->phy);

}

void Ble_LlcpPopulateChannelRemapTable(gpHal_ChannelMap_t* pChanMap, UInt8* pUsedChannels)
{
    UIntLoop i;

    // Copy info to the channel map
    MEMCPY(pChanMap->usedChanIds, pUsedChannels, sizeof(pChanMap->usedChanIds));
    MEMSET(pChanMap->remapTable, 0, sizeof(pChanMap->remapTable));

    pChanMap->hopRemapTableLength = 0;
    for(i = 0; i < BLE_DATA_NUMBER_OF_CHANNELS; i++)
    {
        UInt8 arrayIndex = (i >> 3);
        UInt8 arrayBitIndex = i - (arrayIndex << 3);

        if((pChanMap->usedChanIds[arrayIndex] & (1 << arrayBitIndex)) != 0)
        {
            // channel is used ==> add to remap table
            pChanMap->remapTable[pChanMap->hopRemapTableLength] = i;
            pChanMap->hopRemapTableLength++;
        }
    }
}

gpHal_BleChannelMapHandle_t Ble_GetMasterChannelMapHandle(void)
{
    return Ble_GlobalContext.masterChannelMapHandle;
}

void Ble_LlcpPopulateDefaultMasterChannelMap(void)
{
    gpHal_ChannelMap_t chanMap;

    GP_ASSERT_DEV_INT(GP_HAL_BLE_CHANNEL_MAP_INVALID == Ble_GlobalContext.masterChannelMapHandle);
    Ble_GlobalContext.masterChannelMapHandle = gpHal_BleAllocateChannelMapHandle();

    // create a default master channel map - all data channels used
    {
        UInt8 DefaultMasterChannelMap[BLE_CHANNEL_MAP_SIZE];
        MEMSET(DefaultMasterChannelMap, 0xFF, sizeof(DefaultMasterChannelMap));
        DefaultMasterChannelMap[4] = 0x1F;

        Ble_LlcpPopulateChannelRemapTable(&chanMap, DefaultMasterChannelMap);
    }

    // Store the default master channel map (all data channels in use)
    gpHal_BleSetChannelMap(Ble_GlobalContext.masterChannelMapHandle, &chanMap);
}

gpHci_Result_t Ble_LlcpStartConnectionEstablishmentMaster(Ble_IntConnId_t connId, Ble_ConnEstablishParams_t* pConnEstablishParams)
{
    UInt32 firstRealConnEventTs;
    UInt8 nrOfSkippedIntervals=0;
    gpHal_ConnEventInfo_t connEventInfo;
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    // Store the access address used for the connection in the link context
    pContext->accessAddress = pConnEstablishParams->connReqPdu.llData.accessAddress;

    // Store min/max  connection interval and event length
    pContext->ccParams.connIntervalMin = pConnEstablishParams->connIntervalMin;
    pContext->ccParams.connIntervalMax = pConnEstablishParams->connIntervalMax;
    pContext->ccParams.minCELength = pConnEstablishParams->minCELength;
    pContext->ccParams.maxCELength = pConnEstablishParams->maxCELength;

    // Calculate earliest possible start time of first M->S PDU
    firstRealConnEventTs = BleLlcp_GetEarliestAnchorPoint(pConnEstablishParams);

    if(pConnEstablishParams->winOffsetCalculated)
    {
        UInt32 preferredActivityStartTs = gpBleActivityManager_GetNextActivityTs(connId);

        // firstRealConnEventTs holds the earliest allowed start TS
        // preferredActivityStartTs is the one originally selected by the activity manager.
        // Since RT makes an error in calculating the transmit window offset, compensate for it here.
        // This can be done by ensuring preferredActivityStartTs is later than firstRealConnEventTs
        while(gpBle_IsTimestampEarlier(preferredActivityStartTs, firstRealConnEventTs))
        {
            preferredActivityStartTs += BLE_TIME_UNIT_1250_TO_US(pConnEstablishParams->connReqPdu.llData.interval);
        }

        firstRealConnEventTs = preferredActivityStartTs;
    }
    else
    {
        // If winOffset is not calculated, send the first M->S PDU in the middle of the transmitWindow
        firstRealConnEventTs += (BLE_TIME_UNIT_1250_TO_US(pConnEstablishParams->connReqPdu.llData.winSize) >> 1);
    }

    // Note that the masterChannelMapHandle is only updated after all Channel Map Change procedures (on all master links) have been finished
    // In any case, while the newMasterChannelMapHandle is valid, we need to use it
    Ble_LlcpPopulateConnEventInfo(pContext, &connEventInfo, pConnEstablishParams, Ble_LlcpGetLatestChannelMapHandle(Ble_GlobalContext.masterChannelMapHandle), firstRealConnEventTs, 0, nrOfSkippedIntervals, 0);

    connEventInfo.winOffsetCalculated = pConnEstablishParams->winOffsetCalculated;

#ifdef GP_DIVERSITY_DEVELOPMENT
    firstRealConnEventTs += gpBle_VsdMasterConnEstabFirstMToSSignedOffset;
#endif /* GP_DIVERSITY_DEVELOPMENT */

    // This function can return a non-success status code. We don't check it here, since it does not really make a difference for the next steps.
    // The connection is considered to be created, so we need to indicate this to the host.
    // If this step fails, we rely on the supervision timeout to cleanup the connection.
    gpHal_BleUpdateMasterConnection(connId, firstRealConnEventTs, &connEventInfo);

    Ble_LlcpStartConnectionEstablishmentCommon(pContext, pConnEstablishParams, nrOfSkippedIntervals, firstRealConnEventTs);

    return gpHci_ResultSuccess;
}

void Ble_LlcpStartConnectionEstablishmentSlave(Ble_IntConnId_t connId, Ble_ConnEstablishParams_t* pConnEstablishParams)
{
    UInt32 firstRealConnEventTs;
    UInt8 nrOfSkippedIntervals=0;
    gpHal_BleChannelMapHandle_t channelMapHandle;
    gpHal_ConnEventInfo_t connEventInfo;
    gpHal_ChannelMap_t chanMap;
    Ble_LlcpLinkContext_t* pContext;
    UInt16 masterSca;
    UInt16 slaveSca;
    UInt32 windowDuration;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    masterSca = Ble_ScaFieldToMaxPpm(pConnEstablishParams->connReqPdu.llData.sleepClockAccuracy);
    slaveSca = gpHal_GetSleepClockAccuracy();

    pContext->combinedSca = masterSca + slaveSca;

    // Calculate earliest possible start time of first M->S PDU
    firstRealConnEventTs = BleLlcp_GetEarliestAnchorPoint(pConnEstablishParams);

    // Calculate start of first RX window (as slave should start listening a bit sooner)
    firstRealConnEventTs = BleLlcp_GetSlaveFirstRxTimestamp(pContext, firstRealConnEventTs, pConnEstablishParams, &windowDuration);

    channelMapHandle = gpHal_BleAllocateChannelMapHandle();

    GP_ASSERT_DEV_INT(gpHal_BleIsChannelMapValid(channelMapHandle));

    Ble_LlcpPopulateChannelRemapTable(&chanMap, pConnEstablishParams->connReqPdu.llData.channelMap);

    // Store this channel map
    gpHal_BleSetChannelMap(channelMapHandle, &chanMap);

    // Keep handle for later use
    pContext->channelMapHandle = channelMapHandle;

    // Store access address
    pContext->accessAddress = pConnEstablishParams->connReqPdu.llData.accessAddress;

    Ble_LlcpPopulateConnEventInfo(pContext, &connEventInfo, pConnEstablishParams,
                                  channelMapHandle, firstRealConnEventTs,
                                  masterSca, nrOfSkippedIntervals, windowDuration);

    // This function can return a non-success status code. We don't check it here, since it does not really make a difference for the next steps.
    // The connection is considered to be created, so we need to indicate this to the host.
    // If this step fails, we rely on the supervision timeout to cleanup the connection.
    gpHal_BleStartConnection(connId, firstRealConnEventTs, &connEventInfo);

    // Slave has no create connection context, but some of these parameters are used for connection update. Fill in
    pContext->ccParams.connIntervalMin = pContext->intervalUnit;
    pContext->ccParams.connIntervalMax = pContext->intervalUnit;

    // Add this slave connection to the activity manager
    gpBleActivityManager_AddIncomingActivity(pContext->connId, pConnEstablishParams->connReqPdu.llData.interval, BLEACTIVITYMANAGER_DURATION_UNIT_INVALID, firstRealConnEventTs);

    Ble_LlcpStartConnectionEstablishmentCommon(pContext, pConnEstablishParams, nrOfSkippedIntervals, firstRealConnEventTs);
}

void Ble_LlcpStartConnectionEstablishmentCommon(Ble_LlcpLinkContext_t* pContext, Ble_ConnEstablishParams_t* pConnEstablishParams, UInt8 nrOfSkippedIntervals, UInt32 firstRealConnEventTs)
{
    // Store link context
    pContext->intervalUnit = pConnEstablishParams->connReqPdu.llData.interval;
    pContext->latency = pConnEstablishParams->connReqPdu.llData.latency;
    pContext->timeoutUnit = pConnEstablishParams->connReqPdu.llData.timeout;
    pContext->initialCorrelationTs = firstRealConnEventTs;
    pContext->prohibitSlaveLatencyMask = Ble_ProhibitSlaveLatency_NoRestriction;

    if(pContext->masterConnection)
    {
        // The peer address is the advertiser address
        MEMCPY(&pContext->peerAddress, &pConnEstablishParams->connReqPdu.advAddress, sizeof(BtDeviceAddress_t));
        pContext->peerAddressType = pConnEstablishParams->connReqPdu.pduHeader.rxAdd;
    }
    else
    {
        // The peer address is the initiator address
        MEMCPY(&pContext->peerAddress, &pConnEstablishParams->connReqPdu.initAddress, sizeof(BtDeviceAddress_t));
        pContext->peerAddressType = pConnEstablishParams->connReqPdu.pduHeader.txAdd;
    }

    // Limit slave latency until the first ACK is seen
    gpBleLlcp_ProhibitSlaveLatency(pContext->connId, true, Ble_ProhibitSlaveLatency_Llcp);

    // Open connection in all blocks
    gpBleActivityManager_OpenConnection(pContext->connId, firstRealConnEventTs);
    gpBle_DataOpenConnection(pContext->connId, pConnEstablishParams->phy);
    gpBle_DataTxOpenConnection(pContext->connId);
    gpBle_DataTxQueueOpenConnection(pContext->connId);
    gpBle_DataRxOpenConnection(pContext->connId);
    gpBle_DataRxQueueOpenConnection(pContext->connId);
    gpBleLlcpFramework_OpenConnection(pContext->connId);
#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    gpBleLlcpProcedures_CteOpenConnection(pContext->connId);
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */


    gpBle_SetConnectionBandwidthControl(pContext->connId, pContext->ccParams.maxCELength, pContext->intervalUnit);


    GP_LOG_PRINTF("BLE connection %x opened",0,pContext->connId);
}

void Ble_LlcpResetConnection(Ble_LlcpLinkContext_t* pContext, Bool permanentConnection)
{
    if(permanentConnection)
    {
        gpHal_Result_t result;

        result = gpHal_BleStopConnection(pContext->connId);
        // If this command fails, there is something seriously wrong with RT, and there is no way to recover without side-effects
        GP_ASSERT_SYSTEM(result == gpHal_ResultSuccess);
    }

    if (!pContext->masterConnection)
    {
        if(permanentConnection)
        {
            gpHal_BleFreeChannelMapHandle(pContext->channelMapHandle);
            pContext->channelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;
        }
        Ble_GlobalContext.nrOfSlaveConnections--;
    }
    else
    {
        Ble_GlobalContext.nrOfMasterConnections--;
    }

    if(permanentConnection)
    {
        // We cannot free the link context here for permanent connections, as this might corrupt the framework handling
        // So we schedule the free-ing of the link context (allows the current functions/stack to terminate while still having access to the connection context).
        gpSched_ScheduleEventArg(0, Ble_StopConnectionFollowUp, pContext);
    }
    else
    {
        // Tmp connections can be freed right away
        Ble_StopConnectionFollowUp((void*)pContext);
    }
}

void Ble_StopConnectionFollowUp(void* pArgs)
{
    Ble_LlcpLinkContext_t* pContext = (Ble_LlcpLinkContext_t*)pArgs;

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(Ble_LlcpIsConnectionAllocated(pContext->connId));

    // re-add device to the white list
    if(pContext->masterConnection)
    {
        gpBleAddressResolver_UpdateFilterAcceptListEntryState(pContext->peerAddressType?1:0, &pContext->peerAddress, GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_INITIATING_VALID_MASK, true);
    }
    else
    {
        gpBleAddressResolver_UpdateFilterAcceptListEntryState(pContext->peerAddressType?1:0, &pContext->peerAddress, GPHAL_ENUM_FILTER_ACCEPT_LIST_ENTRY_ADVERTISING_VALID_MASK, true);
    }

    Ble_FreeLinkContext(pContext->connId);
}
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
void Ble_PrepareConnectionComplete(gpHci_LEEnhancedConnectionCompleteEventParams_t* pConnCompleteParams, gpHci_ConnectionHandle_t connHandle, gpHci_ConnectionRole_t role, Ble_ConnEstablishParams_t* pConnEstablishParams)
#else
void Ble_PrepareConnectionComplete(gpHci_LEConnectionCompleteEventParams_t* pConnCompleteParams, gpHci_ConnectionHandle_t connHandle, gpHci_ConnectionRole_t role, Ble_ConnEstablishParams_t* pConnEstablishParams)
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
{
    pConnCompleteParams->connectionHandle = connHandle;
    pConnCompleteParams->role = role;
    gpHci_AdvPeerAddressType_t advAddrType;

    BtDeviceAddress_t *pPeerAddr;
#ifdef GP_COMP_BLERESPRADDR
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
    Bool isPeerResolveLLRpa;
    Bool isLocalResolveLLRpa;
    BtDeviceAddress_t *pLocalAddr;
#endif //GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#endif // GP_COMP_BLERESPRADDR

    if (role == gpHci_ConnectionRoleSlave)
    {
        pPeerAddr = &pConnEstablishParams->connReqPdu.initAddress;
#ifdef GP_COMP_BLERESPRADDR
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
        pLocalAddr = &pConnEstablishParams->connReqPdu.advAddress;
        isPeerResolveLLRpa = BLE_RES_PR_SRC_IS_LL_RESOLVED_RPA(&pConnEstablishParams->rpaInfo);
        isLocalResolveLLRpa = BLE_RES_PR_DST_IS_LL_RESOLVED_RPA(&pConnEstablishParams->rpaInfo);
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#endif // GP_COMP_BLERESPRADDR
        advAddrType = BLE_HAL_ADDR_BIT_TO_ADVPEER_ADDR_TYPE(pConnEstablishParams->connReqPdu.pduHeader.txAdd);
    }
    else
    {
        pPeerAddr = &pConnEstablishParams->connReqPdu.advAddress;
#ifdef GP_COMP_BLERESPRADDR
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
        pLocalAddr = &pConnEstablishParams->connReqPdu.initAddress;
        isPeerResolveLLRpa = BLE_RES_PR_DST_IS_LL_RESOLVED_RPA(&pConnEstablishParams->rpaInfo);
        isLocalResolveLLRpa = BLE_RES_PR_SRC_IS_LL_RESOLVED_RPA(&pConnEstablishParams->rpaInfo);
#endif //GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#endif // GP_COMP_BLERESPRADDR
        advAddrType = BLE_HAL_ADDR_BIT_TO_ADVPEER_ADDR_TYPE(pConnEstablishParams->connReqPdu.pduHeader.rxAdd);
    }

    MEMCPY(&pConnCompleteParams->peerAddress.addr, pPeerAddr->addr, sizeof(BtDeviceAddress_t));
    pConnCompleteParams->peerAddressType = BLE_ADVPEER_ADDR_TYPE_TO_INITPEER_ADDR_TYPE(advAddrType, false);

#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
    MEMSET(pConnCompleteParams->peerPrivateAddress.addr, 0, sizeof(BtDeviceAddress_t));
#else
    MEMCPY(&pConnCompleteParams->peerAddress.addr, pPeerAddr->addr, sizeof(BtDeviceAddress_t));
    pConnCompleteParams->peerAddressType = BLE_ADVPEER_ADDR_TYPE_TO_INITPEER_ADDR_TYPE(advAddrType, false);
#endif //GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE

#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#ifdef GP_COMP_BLERESPRADDR
    if (isPeerResolveLLRpa)
    {
        Bool halAddrBit = false;
        /* Probably better to have the peer identity already in the rpaInfo struct -> more consistent (would we still need the handle?) */
        Bool result = gpHal_BleRpa_GetPeerIdentity(&pConnCompleteParams->peerAddress, &halAddrBit, pConnEstablishParams->rpaInfo.rpaHandle);
        if(!result)
        {
            // Need to handle `false` return value
            pConnCompleteParams->status = gpHci_ResultInvalid;
            return;
        }
        advAddrType = BLE_HAL_ADDR_BIT_TO_ADVPEER_ADDR_TYPE(halAddrBit);
        MEMCPY(pConnCompleteParams->peerPrivateAddress.addr, pPeerAddr->addr, sizeof(BtDeviceAddress_t));
        pConnCompleteParams->peerAddressType = BLE_ADVPEER_ADDR_TYPE_TO_INITPEER_ADDR_TYPE(advAddrType, true);
        GP_LOG_PRINTF("is_rpa=%x is_peer_resolved=%x %x",0,
            BLE_RES_PR_IS_RPA_ADDR(advAddrType, pPeerAddr),
            BLE_RES_PR_IS_VALID_HANDLE(pConnEstablishParams->rpaInfo.rpaHandle),
            pConnCompleteParams->peerAddressType
        );
    }
#endif // GP_COMP_BLERESPRADDR
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE

#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
#ifdef GP_COMP_BLERESPRADDR
    if (isLocalResolveLLRpa)
    {
        MEMCPY(pConnCompleteParams->localPrivateAddress.addr, pLocalAddr->addr, sizeof(BtDeviceAddress_t));
    }
    else
#endif // GP_COMP_BLERESPRADDR
    {
        MEMSET(pConnCompleteParams->localPrivateAddress.addr, 0, sizeof(BtDeviceAddress_t));
    }
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE

    pConnCompleteParams->connInterval = pConnEstablishParams->connReqPdu.llData.interval;
    pConnCompleteParams->connLatency = pConnEstablishParams->connReqPdu.llData.latency;
    pConnCompleteParams->supervisionTo = pConnEstablishParams->connReqPdu.llData.timeout;
    // Version 5.3 | Vol 4, Part E, 7.7.65.1, The Central_Clock_Accuracy parameter is only
    // valid for a Peripheral. On a Central, this parameter shall be set to 0x00.
    pConnCompleteParams->masterClockAccuracy = role == gpHci_ConnectionRoleSlave ?
        pConnEstablishParams->connReqPdu.llData.sleepClockAccuracy : 0;
}

gpHci_Result_t gpBleLlcp_IsHostConnectionHandleValid(gpHci_ConnectionHandle_t connHandle)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LinkInfo[i].connId != BLE_CONN_HANDLE_INVALID)
        {
            if(Ble_LinkInfo[i].hciHandle == BLE_CONN_HANDLE_HANDLE_GET(connHandle))
            {
                return gpHci_ResultSuccess;
            }
        }
    }

    return gpHci_ResultUnknownConnectionIdentifier;
}


void gpBleLlcp_GetConnectionChannelMap(Ble_IntConnId_t connId, gpHci_ChannelMap_t* pChannelMap)
{
    gpHal_BleChannelMapHandle_t ChannelMapHandle;
    gpHal_ChannelMap_t ChanMap;
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    if (pContext->masterConnection)
    {
        ChannelMapHandle = Ble_GlobalContext.masterChannelMapHandle;
    }
    else
    {
        ChannelMapHandle = pContext->channelMapHandle;
    }

    gpHal_BleGetChannelMap(ChannelMapHandle, &ChanMap);
    GP_ASSERT_DEV_INT(5 == sizeof(ChanMap.usedChanIds));
    MEMCPY(pChannelMap->channels, ChanMap.usedChanIds, sizeof(ChanMap.usedChanIds));
}


#ifdef GP_DIVERSITY_DEVELOPMENT
void Ble_SetVsdMasterConnEstabFirstMToSSignedOffset(Int16 SignedOffset)
{
    gpBle_VsdMasterConnEstabFirstMToSSignedOffset = SignedOffset;
}

Int16 Ble_getVsdMasterConnEstabFirstMToSSignedOffset(void)
{
    return gpBle_VsdMasterConnEstabFirstMToSSignedOffset;
}

void Ble_SetVsdArtificialDriftAsSignedNbrMicrosec(Int16 SignedDrift)
{
    gpBle_VsdArtificialDriftAsSignedNbrMicrosec = SignedDrift;
}

Int16 Ble_GetVsdArtificialDriftAsSignedNbrMicrosec(void)
{
    return gpBle_VsdArtificialDriftAsSignedNbrMicrosec;
}
#endif /* GP_DIVERSITY_DEVELOPMENT */

void gpBle_SetVsdfixedRxWindowThresholdParam(UInt16 threshold)
{
    gpBle_VsdfixedRxWindowThreshold = threshold;
}
#ifdef GP_DIVERSITY_DEVELOPMENT
void Ble_SetVsdAccessCode(Bool SetNotClear, UInt32 AccessCode)
{
    gpBle_VsdExternalAccessCodeValid = SetNotClear;
    gpBle_VsdExternalAccessCode = AccessCode;
}
#endif /* GP_DIVERSITY_DEVELOPMENT */


UInt32 BleLlcp_GetEarliestAnchorPoint(Ble_ConnEstablishParams_t* pConnEstablishParams)
{
    UInt32 firstRealConnEventTs;

    // Start of (AUX_)CONNECT_REQ
    firstRealConnEventTs = pConnEstablishParams->timestamp;
    // Add duration of (AUX_)CONNECT_REQ
    firstRealConnEventTs += gpBleDataCommon_GetPacketDurationUs(BLE_CONN_REQ_PAYLOAD_LENGTH, pConnEstablishParams->phy, false);
    // Add blackout time
    firstRealConnEventTs += BLE_CONNECTION_LEGACY_TRANSMIT_BLACKOUT_US;
    // Add transmitWindowOffset
    firstRealConnEventTs += BLE_TIME_UNIT_1250_TO_US(pConnEstablishParams->connReqPdu.llData.winOffset);

    GP_LOG_PRINTF("conn req ts: %lx, duration: %lu",0, (unsigned long)pConnEstablishParams->timestamp, (unsigned long)gpBleDataCommon_GetPacketDurationUs(BLE_CONN_REQ_PAYLOAD_LENGTH, pConnEstablishParams->phy, false));
    GP_LOG_PRINTF("diff: %lu",0, (unsigned long)firstRealConnEventTs-pConnEstablishParams->timestamp);

    return firstRealConnEventTs;
}

UInt32 BleLlcp_GetSlaveFirstRxTimestamp(Ble_LlcpLinkContext_t* pContext, UInt32 earliestAnchorPoint, Ble_ConnEstablishParams_t* pConnEstablishParams, UInt32* pWindowDuration)
{

    UInt32 winSizeUs = BLE_TIME_UNIT_1250_TO_US(pConnEstablishParams->connReqPdu.llData.winSize);
    UInt32 winOffsetUs = BLE_TIME_UNIT_1250_TO_US(pConnEstablishParams->connReqPdu.llData.winOffset);
    UInt32 blackoutUs;

    blackoutUs = BLE_CONNECTION_LEGACY_TRANSMIT_BLACKOUT_US;

    return BleLlcp_CompensateAfterBlackout(pContext, earliestAnchorPoint, blackoutUs, winOffsetUs, winSizeUs, pWindowDuration);
}

UInt32 BleLlcp_CompensateAfterBlackout(Ble_LlcpLinkContext_t* pContext, UInt32 earliestAnchorPoint, UInt32 blackoutUs, UInt32 winOffsetUs, UInt32 winSizeUs, UInt32* pWindowDuration)
{
    UInt32 elapsedUsSinceAnchor;
    UInt32 tmpWindowDurationUs = 0;
    UInt32 drift;

    elapsedUsSinceAnchor = winOffsetUs + blackoutUs;

    GP_ASSERT_DEV_INT(32 <= winSizeUs); /* Needed by RT to open a window */

    // For slave, anchor point should be a bit before start of transmit window (account for window widening)
    drift = Ble_DriftCompensation(pContext->combinedSca, elapsedUsSinceAnchor);
    tmpWindowDurationUs = winSizeUs + 2 * (GP_BLE_T_JITTER_ACTIVE_TOTAL + drift);

    earliestAnchorPoint += (winSizeUs >> 1);
    earliestAnchorPoint -= (tmpWindowDurationUs >> 1);

    *pWindowDuration = tmpWindowDurationUs;

    return earliestAnchorPoint;
}

void BleLlcp_StartInitialProcedures(void* pArg)
{
    Ble_LlcpLinkContext_t* pContext;
    gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

    pContext = (Ble_LlcpLinkContext_t*) pArg;

    if(pContext == NULL || !Ble_LlcpIsConnected(pContext->connId))
    {
       // No valid link context found, return
        GP_ASSERT_DEV_INT(false);
        return;
    }

#ifdef GP_DIVERSITY_DEVELOPMENT
    if(gpBle_VsdAutomaticFeatureExchangeEnabled)
#endif /* GP_DIVERSITY_DEVELOPMENT  */
    {
        MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));

        startDescriptor.procedureId = gpBleLlcp_ProcedureIdFeatureExchange;
        startDescriptor.controllerInit = true;

        // Request the remote for its supported features
        gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
    }

}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcp_Init(gpHal_BleCallbacks_t* pCallbacks)
{
    UIntLoop i;

    Ble_GlobalContext.masterChannelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;
    Ble_GlobalContext.nrOfSlaveConnections = 0;
    Ble_GlobalContext.nrOfMasterConnections = 0;

    // Set link context to invalid values
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        Ble_LinkInfo[i].connId = BLE_CONN_HANDLE_INVALID;
        Ble_LinkInfo[i].connectionCompleteBufferHandle = GP_BLE_EVENT_HANDLE_IDX_INVALID;

    }
}

void gpBleLlcp_Reset(Bool firstReset)
{
    UIntLoop i;

    GP_LOG_PRINTF("LLCP reset: %x",0,firstReset);

    if(Ble_GlobalContext.masterChannelMapHandle != GP_HAL_BLE_CHANNEL_MAP_INVALID)
    {
        gpHal_BleFreeChannelMapHandle(Ble_GlobalContext.masterChannelMapHandle);
        Ble_GlobalContext.masterChannelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;
    }

    // Reset all permanent connections (non-permanent connections are reset by the services that allocated them)
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LlcpIsConnectionCreated(i))
        {
            // Reset all permanent connections without notifying the host
            Ble_LlcpResetConnection(&Ble_LinkInfo[i], true);
        }
    }

    Ble_LlcpPopulateDefaultMasterChannelMap();

#ifdef GP_DIVERSITY_DEVELOPMENT
    gpBle_VsdMasterConnEstabFirstMToSSignedOffset = 0;
#if GP_DIVERSITY_BLECONFIG_VERSION_ID == gpBleConfig_BleVersionId_4_0
    // No need to exchange features on a 4.0 device (also needed for BITE tests)
    gpBle_VsdAutomaticFeatureExchangeEnabled = false;
#else
    gpBle_VsdAutomaticFeatureExchangeEnabled = true;
#endif
    gpBle_VsdArtificialDriftAsSignedNbrMicrosec = 0;
#endif /* GP_DIVERSITY_DEVELOPMENT */
    gpBle_VsdfixedRxWindowThreshold = BLE_FIXED_WD_THRESHOLD_DEFAULT;

#ifdef GP_DIVERSITY_DEVELOPMENT
    gpBle_VsdExternalAccessCodeValid = false;
    gpBle_VsdExternalAccessCode = 0;
#endif /* GP_DIVERSITY_DEVELOPMENT */

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    gpBleLlcpProcedures_CteReset(false);
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */
}



gpHci_Result_t gpBleLlcp_AllocateConnectionSlave(BtDeviceAddress_t* pPeerAddress, gpHci_InitPeerAddressType_t peerAddressType, Ble_IntConnId_t* pConnId)
{
    Ble_LlcpLinkContext_t* pContext;

    *pConnId = BLE_CONN_HANDLE_INVALID;

    if(Ble_GlobalContext.nrOfSlaveConnections >= BLE_LLCP_MAX_NR_OF_SLAVE_CONNECTIONS)
    {
        // We have reached the maximum supported number of slave connections
        return gpHci_ResultConnectionLimitExceeded;
    }

    // check if we have room for a new connection
    pContext = Ble_AllocateLinkContext();

    if(pContext == NULL)
    {
        GP_LOG_PRINTF("No more room to allocate slave connection",0);
        // No more room for a new connection
        return gpHci_ResultMemoryCapacityExceeded;
    }

    pContext->masterConnection = false;
    MEMCPY(&pContext->peerAddress, pPeerAddress, sizeof(BtDeviceAddress_t));
    pContext->peerAddressType = peerAddressType;
    GP_LOG_PRINTF("%02x:%02x:%02x:%02x:%02x:%02x",0,pContext->peerAddress.addr[0],pContext->peerAddress.addr[1],
                         pContext->peerAddress.addr[2],pContext->peerAddress.addr[3],
                         pContext->peerAddress.addr[4],pContext->peerAddress.addr[5]);

    // Not available yet on the slave side, initialize to max
    pContext->ccParams.minCELength = BLE_CE_LENGTH_MAX;

    if(Ble_DataTxQueueVsdIsDefaultCBEnabled())
    {
        pContext->ccParams.maxCELength = 0;
    }
    else
    {
        pContext->ccParams.maxCELength = BLE_CE_LENGTH_MAX;
    }

    Ble_GlobalContext.nrOfSlaveConnections++;
    pContext->connectionStatus = gpBleLlcp_ConnectionStatus_Connecting;
    *pConnId =  pContext->connId;

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBleLlcp_AllocateConnectionMaster(BtDeviceAddress_t* pPeerAddress, gpHci_InitPeerAddressType_t peerAddressType, Ble_IntConnId_t* pConnId)
{
    Ble_LlcpLinkContext_t* pContext;
    *pConnId = BLE_CONN_HANDLE_INVALID;

    if(Ble_GlobalContext.nrOfMasterConnections >= BLE_LLCP_MAX_NR_OF_CONNECTIONS)
    {
        // We have reached the maximum supported number of master connections
        GP_LOG_PRINTF("Maximum number of master connections reached", 0);
        return gpHci_ResultConnectionLimitExceeded;
    }

    // check if we have room for a new connection
    pContext = Ble_AllocateLinkContext();

    if(pContext == NULL)
    {
        // No room for a new connection
        GP_LOG_PRINTF("No link context available",0);
        return gpHci_ResultMemoryCapacityExceeded;
    }

    pContext->masterConnection = true;

    MEMCPY(&pContext->peerAddress, pPeerAddress, sizeof(BtDeviceAddress_t));
    pContext->peerAddressType = peerAddressType;
    Ble_GlobalContext.nrOfMasterConnections++;
    pContext->connectionStatus = gpBleLlcp_ConnectionStatus_Connecting;

    *pConnId = pContext->connId;
    return gpHci_ResultSuccess;
}

void Ble_LlcpFreeConnectionMaster(Ble_IntConnId_t connId)
{
    Ble_FreeLinkContext(connId);
    Ble_GlobalContext.nrOfMasterConnections--;
}

void gpBleLlcp_FreeConnectionSlave(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    if(pContext == NULL)
    {
        // Should not happen
        GP_ASSERT_DEV_INT(false);
        return;
    }

    Ble_LlcpResetConnection(pContext, false);
}


Bool Ble_LlcpIsMasterConnection(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    return Ble_LinkInfo[connId].masterConnection;
}

Bool Ble_LlcpIsConnectionAllocated(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    return Ble_LinkInfo[connId].connId == connId;
}

Bool Ble_LlcpIsConnectionCreated(Ble_IntConnId_t connId)
{
    if(!BLE_IS_INT_CONN_HANDLE_VALID(connId))
    {
        return false;
    }

    return (Ble_LlcpIsConnectionAllocated(connId) && Ble_LlcpIsConnected(connId));
}

Ble_IntConnId_t gpBleLlcp_HciHandleToIntHandle(gpHci_ConnectionHandle_t hciHandle)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LinkInfo[i].hciHandle == BLE_CONN_HANDLE_HANDLE_GET(hciHandle))
        {
            return Ble_LinkInfo[i].connId;
        }
    }

    return BLE_CONN_HANDLE_INVALID;
}

gpHci_ConnectionHandle_t gpBleLlcp_IntHandleToHciHandle(Ble_IntConnId_t connId)
{
    if(connId < BLE_LLCP_MAX_NR_OF_CONNECTIONS)
    {
        return Ble_LinkInfo[connId].hciHandle;
    }

    return 0xFFFF;
}

void gpBleLlcp_FinishConnectionCreation(Ble_IntConnId_t connId, Bool sendConnectionComplete, Ble_ConnEstablishParams_t* pConnEstablishParams)
{
    Ble_LlcpLinkContext_t* pContext;

#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
    gpHci_LEEnhancedConnectionCompleteEventParams_t connCompleteParams;
    MEMSET(&connCompleteParams,0, sizeof(gpHci_LEEnhancedConnectionCompleteEventParams_t));
#else
    gpHci_LEConnectionCompleteEventParams_t connCompleteParams;
    MEMSET(&connCompleteParams,0, sizeof(gpHci_LEConnectionCompleteEventParams_t));
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    if(pConnEstablishParams != NULL)
    {

        GP_LOG_PRINTF("Start conn establish m: %x",0, pContext->masterConnection);

        // Schedule execution of initial LL control procedures
        gpSched_ScheduleEventArg(0, BleLlcp_StartInitialProcedures, pContext);

        pContext->hciHandle = gpBle_AllocateHciConnectionHandle();

        if(pContext->masterConnection)
        {
            Ble_LlcpStartConnectionEstablishmentMaster(connId, pConnEstablishParams);
        }
        else
        {
            Ble_LlcpStartConnectionEstablishmentSlave(connId, pConnEstablishParams);
        }
        pContext->connectionStatus = gpBleLlcp_ConnectionStatus_Connected;
        connCompleteParams.status = gpHci_ResultSuccess;

        Ble_PrepareConnectionComplete(&connCompleteParams, pContext->hciHandle, !pContext->masterConnection, pConnEstablishParams);
    }
    else
    {
        MEMCPY(&connCompleteParams.peerAddress, &pContext->peerAddress, sizeof(BtDeviceAddress_t));
        pContext->connectionStatus = gpBleLlcp_ConnectionStatus_NotConnected;
        connCompleteParams.peerAddressType = pContext->peerAddressType; // use the peerAddressType as part of LE Create Connection command
        pContext->peerAddressType = gpHci_InitPeerAddressType_Invalid;
        if(pContext->masterConnection)
        {
            connCompleteParams.status = gpHci_ResultUnknownConnectionIdentifier;
        }
        else
        {
            connCompleteParams.status = gpHci_ResultAdvertisingTimeout;
        }

        // In case no connection is created, we should still set the correct role in the connection complete
        connCompleteParams.role = !pContext->masterConnection;

        if(!sendConnectionComplete)
        {
            return;
        }
    }

    Ble_TriggerCbConnectionComplete(pContext, &connCompleteParams);

    if(connCompleteParams.status == gpHci_ResultSuccess)
    {
    }
}

void Ble_LlcpUpdateMasterChannelMapHandle(gpHal_BleChannelMapHandle_t chanMapHandle)
{
    GP_ASSERT_DEV_INT(gpHal_BleIsChannelMapValid(chanMapHandle));

    // Only free the old channel map in case it is in use
    if(gpHal_BleIsChannelMapValid(Ble_GlobalContext.masterChannelMapHandle))
    {
        gpHal_BleFreeChannelMapHandle(Ble_GlobalContext.masterChannelMapHandle);
    }

    Ble_GlobalContext.masterChannelMapHandle = chanMapHandle;
}

/* Function to update slave latency in the link context.
 * In case latency was already enabled, the new value is written directly to the gpHal.
 * If not, this will be triggered by a call from gpBleLlcp_ProhibitSlaveLatency.
*/
void gpBleLlcp_UpdateSlaveLatency(Ble_IntConnId_t connId, UInt16 slaveLatency)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    pContext->latency = slaveLatency;

    if(!pContext->masterConnection)
    {
        // Latency should only be updated for slave links
        if(pContext->prohibitSlaveLatencyMask == Ble_ProhibitSlaveLatency_NoRestriction)
        {
            gpHal_BleSetSlaveLatency(connId, slaveLatency);
        }
    }
}

Bool gpBleLlcp_QueueEmptyPacket(Ble_IntConnId_t connId, gpBleData_CteOptions_t* pCteOptions)
{
    gpPd_Loh_t emptyPdLoh;
    gpHci_Result_t result;

    // Get pbm to transmit packet
    emptyPdLoh.handle = gpBle_DataTxQueueAllocatePd(connId, Ble_DataChannelTxQueueCallerData);

    if (GP_PD_INVALID_HANDLE == emptyPdLoh.handle)
    {
        // Could not allocate pd - the protocol/application should retry later
        GP_LOG_PRINTF("could not get resource for empty PDU",0);
        return false;
    }

    emptyPdLoh.offset = GPBLEDATACOMMON_PDU_FOOTER_MAX_OFFSET;
    emptyPdLoh.length = 0;
    result = gpBle_DataTxQueueRequest(connId, emptyPdLoh, Ble_LLID_DataContinuedOrEmpty, pCteOptions);

    return (gpHci_ResultSuccess == result);
}

// Note: this function does not belong in gpHal_Ble : slave latency is a LL property (just like the connection interval, channel map, . . . )
// and this property is independant of the HW implementation.
// We should not push Link Layer intelligence (for managing slave latency or any other link property) into gpHal_Ble
void gpBleLlcp_ProhibitSlaveLatency(Ble_IntConnId_t connId, Bool set, Ble_ProhibitSlaveLatency_Mask_t bitmask_owner)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    if (pContext->masterConnection)
    {
        // slave latency is only managed on slave links
        return;
    }

    if (set)
    {
        if (0 == (pContext->prohibitSlaveLatencyMask & bitmask_owner))
        {
            if ( (Ble_ProhibitSlaveLatency_NoRestriction == pContext->prohibitSlaveLatencyMask) &&
                 (0 != pContext->latency)
               )
            {
                // disable slave latency
                gpHal_BleSetSlaveLatency(pContext->connId, 0);
            }
            pContext->prohibitSlaveLatencyMask |= bitmask_owner;
        }
    }
    else
    {
        if (pContext->prohibitSlaveLatencyMask & bitmask_owner)
        {
            pContext->prohibitSlaveLatencyMask &= (~bitmask_owner);
            if ( (Ble_ProhibitSlaveLatency_NoRestriction == pContext->prohibitSlaveLatencyMask) &&
                 (0 != pContext->latency)
               )
            {
                // allow slave latency
                gpHal_BleSetSlaveLatency(pContext->connId, pContext->latency);
            }
        }
    }
}

Bool gpBleLlcp_IsSlaveLatencyAllowed(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    if(pContext == NULL)
    {
        return false;
    }

    return (pContext->prohibitSlaveLatencyMask == Ble_ProhibitSlaveLatency_NoRestriction);
}

Bool Ble_LlcpAnchorMoveRequested(Ble_IntConnId_t connId, UInt16 interval, UInt16 latency, UInt16 timeout)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    if(interval == pContext->intervalUnit && latency == pContext->latency && timeout == pContext->timeoutUnit)
    {
        return true;
    }

    return false;
}

Bool Ble_LlcpConnParamChangeRequested(Ble_IntConnId_t connId, UInt16 intervalMin, UInt16 intervalMax, UInt16 latency, UInt16 timeout)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    if(intervalMin < pContext->ccParams.connIntervalMin)
    {
        return true;
    }

    if(intervalMax > pContext->ccParams.connIntervalMax)
    {
        return true;
    }

    if(latency != pContext->latency)
    {
        return true;
    }

    if(timeout != pContext->timeoutUnit)
    {
        return true;
    }

    // No change in connection parameters, which means it is an anchor move
    return false;
}

Bool Ble_ConnectionParametersValid(UInt16 connIntervalMin, UInt16 connIntervalMax, UInt16 latency, UInt16 timeout, UInt16 minCe, UInt16 maxCe)
{
    if(!Ble_IsConnectionIntervalValid(connIntervalMin, connIntervalMax))
    {
        return false;
    }

    if(!RANGE_CHECK(latency, BLE_INITIATOR_CONN_LATENCY_RANGE_MIN, BLE_INITIATOR_CONN_LATENCY_RANGE_MAX))
    {
        GP_LOG_PRINTF("connLatency not in range: %x <= %x <= %x",0, BLE_INITIATOR_CONN_LATENCY_RANGE_MIN, latency, BLE_INITIATOR_CONN_LATENCY_RANGE_MAX);
        return false;
    }

    if(!Ble_IsSupervisionToValid(timeout, latency, connIntervalMax))
    {
        return false;
    }

    if(minCe > maxCe)
    {
        GP_LOG_PRINTF("minCELength (%x) > maxCELength (%x)", 0, minCe, maxCe);
        return false;
    }

    return true;
}

Bool Ble_LlcpTxWinOffsetValid(UInt16 offsetUnit, UInt16 intervalUnit)
{
    if(offsetUnit > intervalUnit)
    {
        GP_LOG_PRINTF("winOffset (%x) > interval (%x)",0,offsetUnit,intervalUnit);
        return false;
    }

    return true;
}

Bool Ble_LlcpTxWinSizeValid(UInt16 sizeUnit, UInt16 intervalUnit)
{
    UInt8 winSizeUpper;

    winSizeUpper = min(intervalUnit - 1, BLE_LL_DATA_WIN_SIZE_UPPER);

    if(!RANGE_CHECK(sizeUnit, 1, winSizeUpper))
    {
        GP_LOG_PRINTF("winSize not in range: %x <= %x <= %x",0, 1, sizeUnit, winSizeUpper);
        return false;
    }

    return true;
}

#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
void Ble_TriggerCbConnectionComplete(Ble_LlcpLinkContext_t* pContext, gpHci_LEEnhancedConnectionCompleteEventParams_t* pParams)
#else
void Ble_TriggerCbConnectionComplete(Ble_LlcpLinkContext_t* pContext, gpHci_LEConnectionCompleteEventParams_t* pParams)
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
{
    gpBle_EventBuffer_t* pEventBuf = gpBle_EventHandleToBuffer(pContext->connectionCompleteBufferHandle);

    GP_ASSERT_DEV_EXT(pEventBuf != NULL);

    pEventBuf->eventCode = gpHci_EventCode_LEMeta;
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
    if(gpBleConfig_IsLeMetaEventMasked(gpHci_LEMetaSubEventCodeEnhancedConnectionComplete))
    {
        pEventBuf->payload.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeEnhancedConnectionComplete;
        MEMCPY(&pEventBuf->payload.metaEventParams.params.enhancedConnectionComplete, pParams, sizeof(gpHci_LEEnhancedConnectionCompleteEventParams_t));
    }
    else
    {
        pEventBuf->payload.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeConnectionComplete;
        pEventBuf->payload.metaEventParams.params.connectionComplete.status = pParams->status;
        pEventBuf->payload.metaEventParams.params.connectionComplete.connectionHandle = pParams->connectionHandle;
        pEventBuf->payload.metaEventParams.params.connectionComplete.role = pParams->role;
        pEventBuf->payload.metaEventParams.params.connectionComplete.peerAddressType = pParams->peerAddressType;
        MEMCPY( &pEventBuf->payload.metaEventParams.params.connectionComplete.peerAddress, &pParams->peerAddress, sizeof(BtDeviceAddress_t));
        pEventBuf->payload.metaEventParams.params.connectionComplete.connInterval = pParams->connInterval;
        pEventBuf->payload.metaEventParams.params.connectionComplete.connLatency = pParams->connLatency;
        pEventBuf->payload.metaEventParams.params.connectionComplete.supervisionTo = pParams->supervisionTo;
        // 5.3 | Vol 4, Part E, 7.7.65.10: The Central_Clock_Accuracy parameter is only valid for a Peripheral. On a Central, this parameter shall be set to 0x00.
        pEventBuf->payload.metaEventParams.params.connectionComplete.masterClockAccuracy = pParams->role == gpHci_ConnectionRoleSlave ? pParams->masterClockAccuracy : 0;

    }
#else
    pEventBuf->payload.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeConnectionComplete;
    MEMCPY(&pEventBuf->payload.metaEventParams.params.connectionComplete, pParams, sizeof(gpHci_LEConnectionCompleteEventParams_t));
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
    gpBle_SendEvent(pEventBuf);

    // The event has been queued, so we don't need the event handle anymore.
    // Note that the event buffer will be freed automatically by the sequencer after event transmission
    pContext->connectionCompleteBufferHandle = GP_BLE_EVENT_HANDLE_IDX_INVALID;
}

UInt32 gpBleLlcp_CalculateEarliestAnchorPoint(Ble_IntConnId_t connId, UInt32 correlationTs, UInt32 blackoutUs, UInt32 winOffsetUs, UInt32 winSizeUs, UInt32* pWindowDuration)
{
    UInt32 firstAnchorPoint;

    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);
    GP_ASSERT_DEV_INT(pContext);

    // Calculate first anchor point, to point at the beginning of the transmit window
    firstAnchorPoint = correlationTs + blackoutUs + winOffsetUs;

    return BleLlcp_CompensateAfterBlackout(pContext, firstAnchorPoint, blackoutUs, winOffsetUs, winSizeUs, pWindowDuration);
}

gpBle_AccessAddress_t gpBleLlcp_CreateAccessAddress(void)
{
    gpBle_AccessAddress_t accessAddress;

    // Pointer to accessAddress (for easier calculation)
    UInt8* pAccessAddress = (UInt8*)&accessAddress;
    Bool accessAddressValid = false;

#ifdef GP_DIVERSITY_DEVELOPMENT
    if (gpBle_VsdExternalAccessCodeValid)
    {
        accessAddress = gpBle_VsdExternalAccessCode;
        accessAddressValid = true;
    }
#endif /* GP_DIVERSITY_DEVELOPMENT */

    while(!accessAddressValid)
    {
        // Try a new random sequence
        accessAddress = gpBle_CreateAccessAddress();

        if(Ble_IsAccessAddressUsed(*pAccessAddress))
        {
            // uniqueness amongst all links violated
             GP_LOG_PRINTF("AA 0x%lx invalid: in use by other connection",0,(unsigned long int)accessAddress);
            continue;
        }

        accessAddressValid = true;
    }

    GP_LOG_PRINTF("Using access code 0x%lx",0,(unsigned long int)accessAddress);

    return accessAddress;
}

gpHal_BleChannelMapHandle_t Ble_LlcpGetChannelMapHandle(void)
{
    return Ble_LlcpGetLatestChannelMapHandle(Ble_GetMasterChannelMapHandle());
}


gpHci_Result_t gpBleLlcp_GetAnchorPointFromEventCounterInPast(Ble_IntConnId_t connId, UInt16 eventCounter, UInt32* tAnchor)
{
    gpHal_ConnEventMetrics_t connMetrics;
    UInt32 connIntervalUs;
    UInt16 ecDiff;

    if(!Ble_LlcpIsConnectionCreated(connId))
    {
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    if(gpHal_BleGetConnectionMetrics(connId, &connMetrics) != gpHal_ResultSuccess)
    {
        return gpHci_ResultUnspecifiedError;
    }

    if(!gpBle_IsEcEarlier(eventCounter, connMetrics.eventCounterLastRx))
    {
        return gpHci_ResultInvalidHCICommandParameters;
    }

    connIntervalUs = gpBleLlcp_GetConnIntervalUs(connId);

    ecDiff = BLE_GET_EC_DIFF(eventCounter, connMetrics.eventCounterLastRx);
    *tAnchor = connMetrics.anchorTimeLastRxEvent - (ecDiff * connIntervalUs);

    return gpHci_ResultSuccess;
}

UInt8 gpBleLlcp_GetNumConnections(void)
{
   return  Ble_GlobalContext.nrOfSlaveConnections + Ble_GlobalContext.nrOfMasterConnections;
}

UInt8 gpBleLlcp_GetNumberOfEstablishedConnections(void)
{
    UIntLoop i;
    UInt8 nrOfEstablishedConnections = 0;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LlcpIsConnected(i))
        {
            nrOfEstablishedConnections++;
        }
    }

    return nrOfEstablishedConnections;
}

Bool gpBleLlcp_IsAclHandleInUse(gpHci_ConnectionHandle_t connectionHandle)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LinkInfo[i].connId != BLE_CONN_HANDLE_INVALID && Ble_LinkInfo[i].hciHandle == connectionHandle)
        {
            return true;
        }
    }

    return false;
}

gpHci_ConnectionHandle_t gpBleLlcp_GetFirstAclHandle(void)
{
    UIntLoop i;
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LlcpIsConnected(i))
        {
            return Ble_LinkInfo[i].hciHandle;
        }
    }

    return GP_HCI_CONNECTION_HANDLE_INVALID;
}

gpHci_Result_t gpBleLlcp_DisconnectAclRequest(gpHci_ConnectionHandle_t aclHandle, gpHci_Result_t reason)
{
    Ble_LlcpLinkContext_t* pContext = Ble_GetLinkContext(gpBleLlcp_HciHandleToIntHandle(aclHandle));

    if(!pContext->terminationOngoing)
    {
        gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

        // No termination ongoing, start framework procedure
        GP_LOG_PRINTF("Disconnect connection: %x (reason: %x)",0, aclHandle, reason);

        MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
        startDescriptor.procedureId = gpBleLlcp_ProcedureIdTermination;
        startDescriptor.procedureData.termination.reason = reason;

        return gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
    }
    else
    {
        // Termination is ongoing, just return success and wait for termination trigger to pass
        GP_LOG_PRINTF("Disconnect (controller/remote) already ongoing. Wait",0);
        return gpHci_ResultSuccess;
    }
}

void gpBleLlcp_AclConnectionStop(Ble_IntConnId_t connId, gpHci_Result_t reason)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    GP_LOG_PRINTF("Stop conn: %x (reason: %x)",0,connId, reason);

    // Mark link as termination ongoing
    pContext->terminationOngoing = true;

    gpBle_DataTxCloseConnection(pContext->connId);
    gpBle_DataTxQueueCloseConnection(pContext->connId);
    gpBle_DataRxCloseConnection(pContext->connId);
    gpBle_DataRxQueueCloseConnection(pContext->connId);
    gpBle_DataCloseConnection(pContext->connId);
    gpBleActivityManager_CloseConnection(pContext->connId);
    gpBleLlcpFramework_CloseConnection(pContext->connId);

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    gpBleLlcpProcedures_CteCloseConnection(pContext->connId);
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */


    // Actually cleanup the LLCP context
    Ble_LlcpResetConnection(pContext, true);
}

Bool gpBleLlcp_IsConnectionEncrypted(gpHci_ConnectionHandle_t connectionHandle)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        if(Ble_LlcpIsConnectionAllocated(i) && Ble_LinkInfo[i].hciHandle == connectionHandle)
        {
            return Ble_LinkInfo[i].encryptionEnabled;
        }
    }

    return false;
}

/*****************************************************************************
 *                    Callbacks
 *****************************************************************************/

/* This function is called by gpBleDataChannelTxQueue to notify us when the first ACK (on a non-empty packet) was received.
 * This information will be used to enable slave latency inside a connection. */
void gpBleLlcp_cbAcknowledgeSeen(UInt8 connId)
{
    gpBleLlcp_ProhibitSlaveLatency(connId, false, Ble_ProhibitSlaveLatency_Llcp);
}

/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_ReadRemoteVersionInfo(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    gpHci_ConnectionHandle_t connHandle = pParams->ReadRemoteVersionInfo.connectionHandle;

    GP_LOG_PRINTF("gpBle_ReadRemoteVersion for handle: %x",0,connHandle);

    // Use Command status event to reply to a read remote version info command
    BLE_SET_RESPONSE_EVENT_COMMAND_STATUS(pEventBuf->eventCode);

    result = gpBleLlcp_IsHostConnectionHandleValid(connHandle);

    if(result == gpHci_ResultSuccess)
    {
        Ble_LlcpLinkContext_t* pContext;
        gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

        pContext = Ble_GetLinkContext(gpBleLlcp_HciHandleToIntHandle(connHandle));

        MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
        startDescriptor.procedureId = gpBleLlcp_ProcedureIdVersionExchange;

        result = gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
    }

    return result;
}

gpHci_Result_t gpBle_LeReadRemoteFeatures(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    gpHci_ConnectionHandle_t connHandle = pParams->LeReadRemoteFeatures.connectionHandle;

    GP_LOG_PRINTF("gpBle_LeReadRemoteFeatures for handle: %x",0,connHandle);

    // Use Command status event to reply to a read remote used features command
    BLE_SET_RESPONSE_EVENT_COMMAND_STATUS(pEventBuf->eventCode);

    result = gpBleLlcp_IsHostConnectionHandleValid(connHandle);

    if(result == gpHci_ResultSuccess)
    {
        Ble_LlcpLinkContext_t* pContext;
        gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

        pContext = Ble_GetLinkContext(gpBleLlcp_HciHandleToIntHandle(connHandle));

        MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
        startDescriptor.procedureId = gpBleLlcp_ProcedureIdFeatureExchange;

        result = gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
    }

    return result;
}

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
gpHci_Result_t gpBle_ReadAuthenticatedPayloadTO(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    gpHci_ConnectionHandle_t connHandle = pParams->ReadAuthenticatedPayloadTO.connectionHandle;

    GP_LOG_PRINTF("gpBle_ReadAuthenticatedPayloadTO for handle: %x",0,connHandle);

    // Use Command complete event to reply to a read authenticated payload TO command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = gpBleLlcp_IsHostConnectionHandleValid(connHandle);

    if(result == gpHci_ResultSuccess)
    {
        UInt16 authPayloadTo = gpBle_DataRxQueueReadAuthPayloadTo(gpBleLlcp_HciHandleToIntHandle(connHandle));
        pEventBuf->payload.commandCompleteParams.returnParams.readAuthenticatedPayloadTO.authenticatedPayloadTO = authPayloadTo;
    }

    return result;
}

gpHci_Result_t gpBle_WriteAuthenticatedPayloadTO(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    gpHci_WriteAuthenticatedPayloadTOCommand_t* pPayloadParams = &pParams->WriteAuthenticatedPayloadTO;

    GP_LOG_PRINTF("gpBle_WriteAuthenticatedPayloadTO for handle: %x",0,pPayloadParams->connectionHandle);

    // Use Command complete event to reply to a write authenticated payload TO command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = Ble_WriteAuthenticatedPayloadTOChecker(pPayloadParams);

    if(result == gpHci_ResultSuccess)
    {
        gpBle_DataRxQueueWriteAuthPayloadTo(gpBleLlcp_HciHandleToIntHandle(pPayloadParams->connectionHandle), pPayloadParams->authenticatedPayloadTO);
    }

    return result;
}
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED

gpHci_Result_t gpBle_ReadTransmitPowerLevel(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_ConnectionHandle_t connHandle = pParams->ReadTransmitPowerLevel.connectionHandle;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = gpBleLlcp_IsHostConnectionHandleValid(connHandle);
    pEventBuf->payload.commandCompleteParams.returnParams.advanceTxPower.connectionHandle = BLE_CONN_HANDLE_HANDLE_GET(connHandle);

    if(result == gpHci_ResultSuccess)
    {
        Int8 return_val;
        if (pParams->ReadTransmitPowerLevel.type >= 2)
        {
            return gpHci_ResultInvalidHCICommandParameters;
        }
        if (0 == pParams->ReadTransmitPowerLevel.type)
        {
            return_val = gpHal_BleGetTxPower();
        }
        else
        {
            Int8 MinTxPower;
            Int8 MaxTxPower;
            gpHal_BleGetMinMaxPowerLevels(&MinTxPower, &MaxTxPower);
            return_val = MaxTxPower;
        }
        pEventBuf->payload.commandCompleteParams.returnParams.advanceTxPower.advancedTxPower = return_val;
    }

    return result;
}

gpHci_Result_t gpBle_ReadRSSI(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_ConnectionHandle_t connHandle = pParams->ReadRSSI.connectionHandle;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = gpBleLlcp_IsHostConnectionHandleValid(connHandle);

    if(result == gpHci_ResultSuccess)
    {
        // we (currently) do not cache/average rssi values of received packets
        // spec allows returning a static value "127" - cfr Vol 2 Part E $7.5.4
        // Note that advanceTxPower  has same return param types as ReadRssi
        // (also gpHci_CommandCompleteEvent() takes this approach)
        pEventBuf->payload.commandCompleteParams.returnParams.advanceTxPower.connectionHandle = BLE_CONN_HANDLE_HANDLE_GET(connHandle);
        pEventBuf->payload.commandCompleteParams.returnParams.advanceTxPower.advancedTxPower = 127;
    }

    return result;
}

gpHci_Result_t gpBle_LeReadChannelMap(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_ConnectionHandle_t connHandle = pParams->LeReadChannelMap.connectionHandle;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = gpBleLlcp_IsHostConnectionHandleValid(connHandle);

    if(result == gpHci_ResultSuccess)
    {
        pEventBuf->payload.commandCompleteParams.returnParams.leReadChannelMap.connectionHandle = BLE_CONN_HANDLE_HANDLE_GET(connHandle);
        gpBleLlcp_GetConnectionChannelMap(gpBleLlcp_HciHandleToIntHandle(connHandle), &pEventBuf->payload.commandCompleteParams.returnParams.leReadChannelMap.channelMap);
    }

    return result;
}

gpHci_Result_t gpBle_VsdGenerateAccessAddress(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    pEventBuf->payload.commandCompleteParams.returnParams.accessAddress = gpBleLlcp_CreateAccessAddress();

    return gpHci_ResultSuccess;
}

#ifdef GP_DIVERSITY_DEVELOPMENT
void Ble_SetVsdAutomaticFeatureExchangeEnable(Bool enable)
{
    gpBle_VsdAutomaticFeatureExchangeEnabled = enable;
}
#endif

gpHci_Result_t gpBle_VsdDisableSlaveLatency(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_ConnectionHandle_t HciHandle;
    Ble_IntConnId_t connId;
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    MEMCPY(&HciHandle, &pParams->VsdDisableSlaveLatency.connHandle, sizeof(gpHci_ConnectionHandle_t));
    connId = gpBleLlcp_HciHandleToIntHandle(HciHandle);

    gpBleLlcp_ProhibitSlaveLatency(connId, pParams->VsdDisableSlaveLatency.disabled, Ble_ProhibitSlaveLatency_HostStack);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_VsdEnCBByDefault(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    Ble_EnableCBByDefault(pParams->VsdEnCBByDefault.enable);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_VsdOverruleRemoteMaxRxOctetsAndTime(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_ConnectionHandle_t HciHandle;
    Ble_IntConnId_t connId;
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    MEMCPY(&HciHandle, &pParams->VsdOverruleRemoteMaxRxOctetsAndTime.connHandle, sizeof(gpHci_ConnectionHandle_t));
    connId = gpBleLlcp_HciHandleToIntHandle(HciHandle);

    GP_LOG_SYSTEM_PRINTF("Overrule Max RX Octets Remote : %d",0,pParams->VsdOverruleRemoteMaxRxOctetsAndTime.maxRxOctetsRemote);

    gpBle_SetMaxRxOctetsRemote(connId, pParams->VsdOverruleRemoteMaxRxOctetsAndTime.maxRxOctetsRemote);

    GP_LOG_SYSTEM_PRINTF("Overrule Max RX Time Remote : %d",0,pParams->VsdOverruleRemoteMaxRxOctetsAndTime.maxRxTimeRemote);

    gpBle_SetMaxRxTimeRemote(connId, pParams->VsdOverruleRemoteMaxRxOctetsAndTime.maxRxTimeRemote);

    return gpHci_ResultSuccess;
}
