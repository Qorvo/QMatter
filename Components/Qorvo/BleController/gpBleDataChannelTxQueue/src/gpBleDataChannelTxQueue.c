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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleDataChannelTxQueue/src/gpBleDataChannelTxQueue.c#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEDATACHANNELTXQUEUE

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleDataChannelTxQueue.h"
#include "gpBleSecurityCoprocessor.h"
#include "gpBleDataCommon.h"
#include "gpBleDataTx.h"
#include "gpBleLlcp.h"
#include "gpBle_LLCP_getters.h"
#include "gpBle_defs.h"
#include "gpPoolMem.h"
#include "gpSched.h"
#include "gpEncryption.h"
#include "gpLog.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define MAX_DURATION_15_4_TRANSACTION                       (5000 /* > 4992  == 192 + (6 + 127) * 32 + 12 * 16 + 11 * 32 */)
// FIXME SDP004-2049 /* unit us ; FIXME: move to gpHal_kx_MAC.h */
/* A short time just to start a 15.4 transaction is sufficient */
#define PROCESSING_TIME_15_4                                (50)

#define IS_BANDWIDTH_CONTROL_DISABLED(connId)               (GPHAL_BLE_BANDWIDTH_CONTROL_DISABLED == Ble_DataTxQueueLinkContext[connId].complement_maxCE_Length)

#define LEN_MIN_DATA_WO_PREAMBLE_WO_CRC                     (4 /*addr*/ + 2 /*header*/ +   0 /* data */)

#define DURATION_BLE_TRANSACTION(duration_data_tx, duration_data_rx)      ((T_IFS) + (duration_data_tx) + (T_IFS) + (duration_data_rx))

#ifdef GP_COMP_GPHAL_MAC
#define IS_BEST_EFFORT_MODE_ENABLED()  (gpBle_VsdTimeFor15dot4Operation > 0)
#define IS_15DOT4_DATA_QUEUED()        (!gpHal_IsMacQueueEmpty())
#endif /* GP_COMP_GPHAL_MAC */

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_IS_LLID_VALID(llid)     (llid <= Ble_LLID_Control)

#define BLE_ENCRYPTION_ENABLE(connId)       (Ble_DataTxAttributes.encryptionEnabled |= (1 << connId))
#define BLE_ENCRYPTION_DISABLE(connId)      (Ble_DataTxAttributes.encryptionEnabled &= ~(1 << connId))
#define BLE_ENCRYPTION_IS_ENABLED(connId)   ((Ble_DataTxAttributes.encryptionEnabled & (1 << connId)) != 0)

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    UInt16 llcpPduPending;      // bit field: one per connection
    UInt16 encryptionEnabled;   // bit field: one per connection
} Ble_DataChannelTxAttributes_t;

typedef struct {
    Ble_IntConnId_t connId;
    Bool ackSeen;
    Ble_EmptyQueueCallback_t emptyQueueCallback;
    UInt32 complement_maxCE_Length;
} Ble_DataChannelTxLinkContext_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static Ble_DataChannelTxAttributes_t Ble_DataTxAttributes;

static Ble_DataChannelTxLinkContext_t Ble_DataTxQueueLinkContext[BLE_LLCP_MAX_NR_OF_CONNECTIONS];

#ifdef GP_DIVERSITY_DEVELOPMENT
static Ble_IntConnId_t gpBle_VsdGeneratePacketWithCorruptedMIC_OnConnId;
#endif /* GP_DIVERSITY_DEVELOPMENT */

#ifdef GP_COMP_GPHAL_MAC
static UInt32 gpBle_VsdTimeFor15dot4Operation = (PROCESSING_TIME_15_4 + MAX_DURATION_15_4_TRANSACTION);
static UInt8  gpBle_Queued_QTA_entries = 0;
#endif /* GP_COMP_GPHAL_MAC */

/* When this variable is set to true, complement_maxCE_Length will be set 0 on each connection setup, meaning CB is enabled */
static Bool gpBle_VsdDefaultCBEnabled = false;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void Ble_HalDataConf(UInt8 connId, gpPd_Loh_t pdLoh);
static void Ble_DataTxQueueScheduleCbIfNeeded(Ble_IntConnId_t connId);
static void Ble_DataTxQueueTriggerEmptyQueueCallback(void* pArg);
static UInt32 Ble_UpdateConnectionGuardTime(Ble_IntConnId_t connId, UInt16 lengthNewTxPacket_wo_preamble);
static void Ble_RestoreConnectionGuardTime(Ble_IntConnId_t connId, UInt32 guardTime);
static UInt16 Ble_GetAllowedSlaveLatencyValue(Ble_IntConnId_t connId);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void Ble_HalPurgeConf(UInt8 connId, gpPd_Handle_t handle)
{
    GP_LOG_PRINTF("Ble_HalPurgeConf h 0x%x ",2, handle);

    Ble_RMFreeResource(connId, handle);
}

void Ble_HalDataConf(UInt8 connId, gpPd_Loh_t pdLoh)
{
    UInt16 header;
    UInt16 pdHandle;

    pdHandle = pdLoh.handle;

    gpPd_ReadWithUpdate(&pdLoh, 2, (UInt8*)&header);

    // Free resource and make sure not to use any pdLoh info afterwards
    // NOTE: the pdHandle is used by gpBleLlcpFramework_cbDataConfirm to find
    // the corresponding llcp procedure but is not dereferenced.
    // It needs to be freed here already because otherwise the claim for the
    // next DataTx might fail
    Ble_RMFreeResource(connId, pdLoh.handle);

    // Currently, we do not need/process HalDataConf for packets with length 0
    if (BLE_IS_INT_CONN_HANDLE_VALID(connId) && BLE_DATA_PDU_HEADER_LENGTH_GET(header) > 0)
    {

        if(!Ble_DataTxQueueLinkContext[connId].ackSeen)
        {
            Ble_DataTxQueueLinkContext[connId].ackSeen = true;
            // Trigger callback, notify upper layers that we have seen an ACK
            gpBleLlcp_cbAcknowledgeSeen(connId);
        }

        if(BLE_DATA_PDU_HEADER_LLID_GET(header) == Ble_LLID_Control)
        {
            // Confirm for control PDUs is handled by the LLCP framework
            gpBleLlcpFramework_cbDataConfirm(connId, pdHandle);
        }
        else
        {
            // Confirm for data PDUs is handled by the data tx block
            gpBleDataTx_cbDataConfirm(connId);
        }
    }

    if (BLE_IS_INT_CONN_HANDLE_VALID(connId))
    {
        if(gpHal_BleIsTxQueueEmpty(connId))
        {
            Ble_UpdateConnectionGuardTime(connId, LEN_MIN_DATA_WO_PREAMBLE_WO_CRC);
        }

        // Trigger empty queue callback when needed
        Ble_DataTxQueueScheduleCbIfNeeded(connId);
    }
}

void Ble_DataTxQueueScheduleCbIfNeeded(Ble_IntConnId_t connId)
{
    if(Ble_DataTxQueueLinkContext[connId].emptyQueueCallback != NULL)
    {
        if(gpHal_BleIsTxQueueEmpty(connId))
        {
            gpSched_ScheduleEventArg(0, Ble_DataTxQueueTriggerEmptyQueueCallback, &Ble_DataTxQueueLinkContext[connId]);
        }
    }
}

void Ble_DataTxQueueTriggerEmptyQueueCallback(void* pArg)
{
    Ble_DataChannelTxLinkContext_t* pContext = (Ble_DataChannelTxLinkContext_t*)pArg;

    GP_ASSERT_DEV_INT(pContext->emptyQueueCallback != NULL);

    pContext->emptyQueueCallback(pContext->connId);
}

UInt32 Ble_UpdateConnectionGuardTime(Ble_IntConnId_t connId, UInt16 lengthNewTxPacket_wo_preamble)
{
    gpHci_PhyWithCoding_t phyTx = gpBle_GetEffectivePhyTxTypeWithCoding(connId);
    UInt16 maxQueuedPacketLength = gpHal_BleGetMaxQueuedPacketLength(connId);
    UInt16 DurationMaxQdPacketSize_withPreamble = gpBleDataCommon_GetPacketDurationUs(maxQueuedPacketLength, phyTx, true);
    UInt16 MaxTxPacketDuration = gpBleDataCommon_GetPacketDurationUs(lengthNewTxPacket_wo_preamble, phyTx, true);
    UInt16 MaxRxPacketDuration = gpBle_GetEffectiveMaxRxPacketDuration(connId);

    UInt32 maxPacketPairDurationUs;
    UInt32 extraIdleTimeUs;
    UInt32 oldGuardTime;

    // check if new Tx data packet is longer than max Queued packet size
    if (DurationMaxQdPacketSize_withPreamble > MaxTxPacketDuration)
    {
        MaxTxPacketDuration = DurationMaxQdPacketSize_withPreamble;
    }

    maxPacketPairDurationUs = DURATION_BLE_TRANSACTION(MaxTxPacketDuration, MaxRxPacketDuration);
    extraIdleTimeUs = 0;

    if (IS_BANDWIDTH_CONTROL_DISABLED(connId))
    {
#ifdef GP_COMP_GPHAL_MAC
        // In 15.4 dual mode operation + Best Effort, we also need time to finish some 15.4 transaction(s)
        // Best effort mode can only be active in dual mode configurations (BLE + 15.4)
        // IS_15DOT4_DATA_QUEUED() does not always shows the actual status of the pending MAC packets in QTA queue
        // a pending MAC packet counter is used as a work around to keep track of pending 15.4 TX packets
        if ((IS_BEST_EFFORT_MODE_ENABLED()) && (1 /*IS_15DOT4_DATA_QUEUED()*/) && gpBle_Queued_QTA_entries)
        {
            maxPacketPairDurationUs += gpBle_VsdTimeFor15dot4Operation; // MaxDurationPacketPair is guaranteed to finish before the next connection event
        }
#endif /* GP_COMP_GPHAL_MAC */
    }
    else
    {
        // in Controlled Bandwidth operation, reserve some time for other activities
        extraIdleTimeUs = Ble_DataTxQueueLinkContext[connId].complement_maxCE_Length; // ExtraIdleTime is not guaranteed (it does overlap with the RT-initialization of the next connection event)
    }

    // Get the old guard time, in case we need to restore it later on
    gpHal_GetConnectionGuardTime(connId, &oldGuardTime);
    gpHal_SetConnectionGuardTime(connId, maxPacketPairDurationUs, extraIdleTimeUs, Ble_GetAllowedSlaveLatencyValue(connId));

    return oldGuardTime;
}

void Ble_RestoreConnectionGuardTime(Ble_IntConnId_t connId, UInt32 guardTime)
{
    gpHal_SetConnectionGuardTime(connId, guardTime, 0, Ble_GetAllowedSlaveLatencyValue(connId));
}

UInt16 Ble_GetAllowedSlaveLatencyValue(Ble_IntConnId_t connId)
{
    if(gpBleLlcp_IsSlaveLatencyAllowed(connId))
    {
        return gpBleLlcp_GetLatency(connId);
    }
    else
    {
        return 0;
    }
}

#ifdef GP_COMP_GPHAL_MAC
void MacFrameQueueStatusChanged(void)
{
    UIntLoop connId;
    for(connId = 0; connId < BLE_LLCP_MAX_NR_OF_CONNECTIONS; connId++)
    {
        if (gpBle_IsConnectionOpen(connId))
        {
            Ble_UpdateConnectionGuardTime(connId, LEN_MIN_DATA_WO_PREAMBLE_WO_CRC);
        }
    }
}

// MacFrameQueuedCB may be called from interrupt
void MacFrameQueuedCB(void)
{
    gpBle_Queued_QTA_entries++;
    gpSched_ScheduleEvent(0, MacFrameQueueStatusChanged);
}

// MacFrameUnqueuedCB may be called from interrupt
void MacFrameUnqueuedCB(void)
{
    gpBle_Queued_QTA_entries--;
    gpSched_ScheduleEvent(0, MacFrameQueueStatusChanged);
}
#endif /* GP_COMP_GPHAL_MAC */

void gpBle_cbDataTxQueueResourceAvailable(Ble_IntConnId_t connId)
{
    // Check if there is any link that wants to send LLCP PDU's
    if(Ble_DataTxAttributes.llcpPduPending != 0)
    {
        UIntLoop i = 0;

        while(BIT_TST(Ble_DataTxAttributes.llcpPduPending, i) == 0)
        {
            i++;
        }

        BIT_CLR(Ble_DataTxAttributes.llcpPduPending, i); // clear the bit for the connection that will now get a chance to send its LLCP PDU
        Ble_LlcpResourceAvailableInd(i);
    }
    else
    {
        gpBle_TxResourceAvailableInd(connId); // gpBle_TxResourceAvailableInd knows what to do with an invalid connId
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBle_DataTxQueueInit(gpHal_BleCallbacks_t* pCallbacks)
{
    pCallbacks->cbDataConf = Ble_HalDataConf;
    // unsolicited purge conf
    pCallbacks->cbPurgeConf = Ble_HalPurgeConf;
    // Assert when storage for connections is insufficient
    COMPILE_TIME_ASSERT(8*(sizeof(Ble_DataTxAttributes.llcpPduPending)) >= BLE_LLCP_MAX_NR_OF_CONNECTIONS);
    COMPILE_TIME_ASSERT(8*(sizeof(Ble_DataTxAttributes.encryptionEnabled)) >= BLE_LLCP_MAX_NR_OF_CONNECTIONS);

#ifdef GP_DIVERSITY_DEVELOPMENT
    gpBle_VsdGeneratePacketWithCorruptedMIC_OnConnId = BLE_CONN_HANDLE_INVALID;
#endif /* GP_DIVERSITY_DEVELOPMENT */

#ifdef GP_COMP_GPHAL_MAC
    gpHal_RegisterMacFrameQueuedCallback(MacFrameQueuedCB);
    gpHal_RegisterMacFrameUnqueuedCallback(MacFrameUnqueuedCB);
    gpBle_Queued_QTA_entries = 0;
#endif /* GP_COMP_GPHAL_MAC */

}

void gpBle_DataTxQueueReset(Bool firstReset)
{
    UIntLoop i;

    // Reset global context
    Ble_DataTxAttributes.encryptionEnabled = 0x0;
    Ble_DataTxAttributes.llcpPduPending = 0x0;

    // Reset link context
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        MEMSET(&Ble_DataTxQueueLinkContext[i], 0, sizeof(Ble_DataTxQueueLinkContext[i]));
        Ble_DataTxQueueLinkContext[i].connId = i;
    }

#ifdef GP_COMP_GPHAL_MAC
    gpBle_VsdTimeFor15dot4Operation = (PROCESSING_TIME_15_4 + MAX_DURATION_15_4_TRANSACTION);
    gpBle_Queued_QTA_entries = 0;
#endif /* GP_COMP_GPHAL_MAC */
    gpBle_VsdDefaultCBEnabled = false;
}

void gpBle_DataTxQueueOpenConnection(Ble_IntConnId_t connId)
{
    if(gpBle_VsdDefaultCBEnabled)
    {
        Ble_DataTxQueueLinkContext[connId].complement_maxCE_Length = GPHAL_BLE_BANDWIDTH_CONTROL_ENABLED;
    }
    else
    {
        Ble_DataTxQueueLinkContext[connId].complement_maxCE_Length = GPHAL_BLE_BANDWIDTH_CONTROL_DISABLED;
    }

    Ble_DataTxQueueLinkContext[connId].ackSeen = false;
}

void gpBle_DataTxQueueCloseConnection(Ble_IntConnId_t connId)
{
    BIT_CLR(Ble_DataTxAttributes.llcpPduPending, connId);
    gpBle_DataTxQueueEnableEncryption(connId, false);
}

gpPd_Handle_t gpBle_DataTxQueueAllocatePd(Ble_IntConnId_t connId, Ble_DataChannelTxQueueCaller_t caller)
{
    gpPd_Loh_t pdLoh = {0,0,GP_PD_INVALID_HANDLE};
    UInt8 nrOfAvailableQueueEntries;
    gpHci_Result_t result;

    if(caller == Ble_DataChannelTxQueueCallerData && Ble_DataTxAttributes.llcpPduPending != 0)
    {
        // Data tx service wants to claim pbm, but LLCP procedure is pending. Return invalid handle
        return GP_PD_INVALID_HANDLE;
    }

    nrOfAvailableQueueEntries = gpHal_BleGetNrOfAvailableLinkQueueEntries(connId);

    if(caller == Ble_DataChannelTxQueueCallerLlcp)
    {
        if(nrOfAvailableQueueEntries == 0)
        {
            return GP_PD_INVALID_HANDLE;
        }
    }
    else
    {
        if(nrOfAvailableQueueEntries < 2)
        {
            return GP_PD_INVALID_HANDLE;
        }
    }

    result = Ble_RMGetResource(&pdLoh);

    if(result != gpHci_ResultSuccess)
    {
        if(caller == Ble_DataChannelTxQueueCallerLlcp)
        {
            BIT_SET(Ble_DataTxAttributes.llcpPduPending, connId);
        }
    }

    return pdLoh.handle;
}

gpHci_Result_t gpBle_DataTxQueueRequest(Ble_IntConnId_t connId, gpPd_Loh_t pdLoh, Ble_LLID_t llid, gpBleData_CteOptions_t* pCteOptions)
{
    gpHci_Result_t result;
    UInt8 pdPayloadLength = pdLoh.length;
    gpBle_AccessAddress_t accessAddress;
    UInt8 pduHeader;
    UInt32 oldGuardTime;

    gpHal_BleTxOptions_t txOptions;

    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));
    GP_ASSERT_DEV_INT(BLE_IS_LLID_VALID(llid));

    pduHeader = llid;
    txOptions.supplementalLengthUs = 0;

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    // Add cteInfo when needed
    if(pCteOptions != NULL && pCteOptions->includeCte)
    {
        GP_LOG_PRINTF("include CTE",0);
        UInt8 cteInfo = 0;

        // Update pdu header (add cp-bit)
        BLE_DATA_PDU_HEADER_CP_SET(pduHeader, 0x01);

        // Populate the CTE info field
        BLE_CTEINFO_TIME_SET(cteInfo, pCteOptions->cteDurationUnit);
        BLE_CTEINFO_TYPE_SET(cteInfo, pCteOptions->cteType);

        gpPd_PrependWithUpdate(&pdLoh, 1, &cteInfo);

        txOptions.supplementalLengthUs = pCteOptions->cteDurationUnit*8;
    }

    // Add header (length + LLID)
    if(pCteOptions != NULL && pCteOptions->includeCte)
    {
        UInt8 length = pdLoh.length - 1;

        gpPd_PrependWithUpdate(&pdLoh, 1, &length);
    }
    else
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    {
        UInt8 length = (UInt8) pdLoh.length;
        GP_ASSERT_DEV_INT( pdLoh.length < 255 );
        gpPd_PrependWithUpdate(&pdLoh, 1, &length);
    }

    if ( BLE_ENCRYPTION_IS_ENABLED(connId) &&
         (pdPayloadLength > 0)
       )
    {
        // Encryption enabled on link. Mask header byte for authentication first
        UInt8 maskedHeaderByte = (pduHeader & BLE_DATA_PDU_FIRST_HEADER_BYTE_AUTH_MASK);

        // Add masked header byte for the authentication, update afterwards
        gpPd_PrependWithUpdate(&pdLoh, 1, &maskedHeaderByte);

        result = gpBle_SecurityCoprocessorCcmEncrypt(connId, &pdLoh);

        if(result != gpHci_ResultSuccess)
        {
            return gpHci_ResultHardwareFailure; //result;
        }

        // Authentication/ encryption succesful, now overwrite masked byte with original one
        gpPd_WriteByte(pdLoh.handle, pdLoh.offset, pduHeader);

        // Successfull encryption: update PDU
        gpPd_WriteByte(pdLoh.handle, pdLoh.offset+1, pdPayloadLength + BLE_SEC_MIC_LENGTH);
        pdLoh.length += BLE_SEC_MIC_LENGTH;

#ifdef GP_DIVERSITY_DEVELOPMENT
        if (gpBle_VsdGeneratePacketWithCorruptedMIC_OnConnId == connId)
        {
            GP_LOG_PRINTF("Generated a corrupt PDU",0);
            // connection will be terminated by remote: avoid corrupting frames on a future connection with the same ConnId
            gpBle_VsdGeneratePacketWithCorruptedMIC_OnConnId = BLE_CONN_HANDLE_INVALID;

            // corrupt the 2nd byte of the encrypted payload (or first payload byte when CTEinfo is present)
            // note that an encrypted data packet always has at least 1 + BLE_SEC_MIC_LENGTH payload bytes: so "pdLoh.offset+3" is safe
            gpPd_WriteByte(pdLoh.handle, pdLoh.offset+3, gpPd_ReadByte(pdLoh.handle, pdLoh.offset+3) ^ 0x55);
        }
#endif /* GP_DIVERSITY_DEVELOPMENT */
    }
    else
    {
        // No encryption on link. Just add complete header byte
        gpPd_PrependWithUpdate(&pdLoh, 1, &pduHeader);
    }

    Ble_GetAccessAddress(connId, &accessAddress);

    // Add access address
    gpPd_PrependWithUpdate(&pdLoh, sizeof(gpBle_AccessAddress_t), (UInt8*)&accessAddress);

    // Update connection guard time before queueing the new packet - see also Jira SW-4998 for background info
    oldGuardTime = Ble_UpdateConnectionGuardTime(connId, pdLoh.length);

    if(gpHal_BleAddPduToQueue(connId, pdLoh, &txOptions) != gpHci_ResultSuccess)
    {
        // Queueing failed - e.g. queue is full :
        // This should only happen in rare cases since we check the nbr of available queue entries before a DataTxQueueRequest
        // This may result in a sub-optimal guard time: guard time too long, causing slightly reduced throughput
        // Note:
        //   Restoring the old value does not always prevent the issue: the RT subsystem may already have decided to stop the connection event based on the new value
        //   . . . this should only happen in rare cases: we accept this limitation
        Ble_RestoreConnectionGuardTime(connId, oldGuardTime);
        return gpHci_ResultHardwareFailure;
    }

    return gpHci_ResultSuccess;
}

void gpBle_DataTxQueueEnableEncryption(Ble_IntConnId_t connId, Bool enable)
{
    if(enable)
    {
        BLE_ENCRYPTION_ENABLE(connId);
    }
    else
    {
        BLE_ENCRYPTION_DISABLE(connId);
    }
}

void gpBle_DataTxQueueRegisterEmptyQueueCallback(Ble_IntConnId_t connId, Ble_EmptyQueueCallback_t callback)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));
    GP_ASSERT_DEV_INT(Ble_DataTxQueueLinkContext[connId].emptyQueueCallback == NULL);

    Ble_DataTxQueueLinkContext[connId].emptyQueueCallback = callback;

    Ble_DataTxQueueScheduleCbIfNeeded(connId);
}

void gpBle_DataTxQueueUnregisterEmptyQueueCallback(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    Ble_DataTxQueueLinkContext[connId].emptyQueueCallback = NULL;

    // Remove any previous running instances
    while(gpSched_UnscheduleEventArg(Ble_DataTxQueueTriggerEmptyQueueCallback, &Ble_DataTxQueueLinkContext[connId]));
}

void gpBle_SetConnectionBandwidthControl(Ble_IntConnId_t connId, UInt16 newMaxCELengthUnit, UInt16 currentIntervalUnit)
{
    UInt32 MaxCE_Complement;

    if (BLE_TIME_UNIT_625_TO_US(newMaxCELengthUnit) > BLE_TIME_UNIT_1250_TO_US(currentIntervalUnit))
    {
        GP_LOG_PRINTF("Controlled Bandwidth mode DISABLED || I=0x%lx ||  maxCE = 0x%lx",0,(unsigned long int)currentIntervalUnit, (unsigned long int)newMaxCELengthUnit);
        // ToDo: put link into normal priority
        MaxCE_Complement = GPHAL_BLE_BANDWIDTH_CONTROL_DISABLED;
    }
    else
    {
        // ToDo: put link into high priority
        MaxCE_Complement = BLE_TIME_UNIT_625_TO_US((2*(UInt32)currentIntervalUnit) - (UInt32)newMaxCELengthUnit);
        GP_LOG_PRINTF("Controlled Bandwidth mode ENABLED || I=0x%lx ||  maxCE = 0x%lx || COMPL= 0x%lx",0,
                            (unsigned long int)currentIntervalUnit,
                            (unsigned long int)newMaxCELengthUnit,
                            (unsigned long int)MaxCE_Complement);
    }

    Ble_DataTxQueueLinkContext[connId].complement_maxCE_Length = MaxCE_Complement;

    Ble_UpdateConnectionGuardTime(connId, LEN_MIN_DATA_WO_PREAMBLE_WO_CRC);
}

#ifdef GP_DIVERSITY_DEVELOPMENT
void Ble_SetVsdGeneratePacketWithCorruptedMIC_OnConnId(Ble_IntConnId_t connId)
{
    gpBle_VsdGeneratePacketWithCorruptedMIC_OnConnId = connId;
}
#endif /* GP_DIVERSITY_DEVELOPMENT */

void Ble_SetTimeFor15Dot4(UInt32 timeFor15Dot4)
{
#ifdef GP_COMP_GPHAL_MAC
    gpBle_VsdTimeFor15dot4Operation = timeFor15Dot4;
#endif /* GP_COMP_GPHAL_MAC */
}

void Ble_EnableCBByDefault(Bool enable)
{
    GP_LOG_PRINTF("Controlled Bandwith default = %d",0,enable);

    gpBle_VsdDefaultCBEnabled = enable;
}

Bool Ble_DataTxQueueVsdIsDefaultCBEnabled(void)
{
    return gpBle_VsdDefaultCBEnabled;
}
