/*
 *   Copyright (c) 2015-2016, GreenPeak Technologies
 *   Copyright (c) 2017, Qorvo Inc
 *
 *
 *   Implementation of gpBleDataTx
 *
 *   This software is owned by Qorvo Inc
 *   and protected under applicable copyright laws.
 *   It is delivered under the terms of the license
 *   and is intended and supplied for use solely and
 *   exclusively with products manufactured by
 *   Qorvo Inc.
 *
 *
 *   THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *   CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *   IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *   LIMITED TO, IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *   QORVO INC. SHALL NOT, IN ANY
 *   CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *   INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *   FOR ANY REASON WHATSOEVER.
 *
 *   $Header$
 *   $Change$
 *   $DateTime$
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// #define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEDATATX

#include "gpBleDataTx.h"
#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleDataChannelTxQueue.h"
#include "gpBleDataCommon.h"
#include "gpBleLlcp.h"
#include "gpBle_defs.h"
#include "gpSched.h"
#include "gpLog.h"
#ifdef GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX
#include "gpBleLlcpProcedures_ConstantToneExtension.h"
#endif /* GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX */
#ifdef GP_DIVERSITY_DEVELOPMENT
#include "gpRandom.h"
#endif
#include "gpBle_Connections.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#if defined(GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX) && (!defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED))
#error "When supporting unsolicited CTE Tx, also flag GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED must be set"
#endif /* GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX && !GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

#define BLE_NUM_TX_DATA_BUFFERS                 GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_TX
#define INVALID_SEQ_NBR 0xFFFF

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
extern void hciCoreTxAclComplete(void *pConn, void *pData);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#define BLE_PB_FLAG_PREVIOUS_WAS_EMPTYSTART    (8)
typedef struct {
    UInt8               *start;
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    void*               pConn;
    void*               pWsfBuff;
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
    gpHci_ConnectionHandle_t connHandle;
    Ble_IntConnId_t     cid;
    Ble_LLID_t          llid;
    UInt16              totalLength;
    UInt16              actualLength;
    UInt16              SequenceNbr;
    UInt8               pb_flag;
    Bool                paused;
    UInt8               NumFrags;
} Ble_BufferAdmin_t;

typedef struct {
    Ble_IntConnId_t     connId;
    UInt8               NumQueuedPds;
    UInt8               pauseCounter;
} Ble_DataTxConnAdmin_t;

#ifdef GP_DIVERSITY_DEVELOPMENT
typedef struct {
    gpHci_VsdSetDataPumpParametersCommand_t vsdParams;
    UInt16 currentpacketCount;
    UInt16 confirmPacketCount;
    Bool enabled;
    UInt8 incrementalPerPacketStartByte;
} Ble_DataPumpParameters_t;
#endif //GP_DIVERSITY_DEVELOPMENT

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// The data TX context for all connections
static Ble_BufferAdmin_t         Ble_BufferAdmin[BLE_NUM_TX_DATA_BUFFERS];
static Ble_IntConnId_t           Ble_activeCid;
static Ble_DataTxConnAdmin_t     Ble_ConnAdmin[BLE_LLCP_MAX_NR_OF_CONNECTIONS];
static UInt16 Ble_HCI_PacketSeqNbrCount;

#ifdef GP_DIVERSITY_DEVELOPMENT
static Ble_DataPumpParameters_t Ble_DataPumpParameters;
static UInt8 BleDataTx_VsdPacketData[GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_TX];
#endif //GP_DIVERSITY_DEVELOPMENT

#ifndef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
static UInt8 Ble_DataTXPool[BLE_NUM_TX_DATA_BUFFERS][GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_TX];
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static UInt8 Ble_DataFindBufferWithSmallestSeqNbr(UInt16 PreviousSmaller);
static void Ble_DataHandleSequenceNbrWrap(void);
static UInt16 Ble_DataCalculateMaxAllowedTxPduSize(Ble_IntConnId_t connectionId, UInt8 CteDuration);
static gpHci_Result_t Ble_DoDataTxRequest(UInt8 pool, gpPd_Loh_t *pdLoh);
static UInt8 Ble_DataFindNextBufferForConnection(UInt8 CurrentPool, Ble_IntConnId_t currentCid);
static Bool Ble_DataCheckLinkAllowance(Ble_IntConnId_t cid);
static Bool Ble_DataCheckAndGetValidTxResource(gpPd_Loh_t *pdLoh, Ble_IntConnId_t connId);
static void Ble_ReleaseDataTxBuffer(UInt8 BufferId);

static void Ble_SendDataBufferOverflowEvent(gpHci_LinkType_t linkType);

static void Ble_DataTxUnpauseFollowup(void* pArgs);

// Checker/action functions
#ifdef GP_DIVERSITY_DEVELOPMENT
static gpHci_Result_t Ble_VsdSetDataPumpParametersChecker(gpHci_VsdSetDataPumpParametersCommand_t* pParams, Bool enableCheck);
static gpHci_Result_t Ble_VsdSetDataPumpEnableChecker(Bool enable);
static gpHci_Result_t Ble_VsdSetDataPumpEnableAction(Bool enable);
static void Ble_VsdDataPumpTriggerNextTransmission(void);
static void Ble_VsdDataPumpInitPayload(UInt8* pPayload);
#endif //GP_DIVERSITY_DEVELOPMENT

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void Ble_SendDataBufferOverflowEvent(gpHci_LinkType_t linkType)
{
    gpHci_EventCbPayload_t payload;

    payload.linkType = linkType;

    gpBle_ScheduleEvent(0, gpHci_EventCode_DataBufferOverflow, &payload);
}


void Ble_DataTxUnpauseFollowup(void* pArgs)
{
    Ble_DataTxConnAdmin_t* pContext = (Ble_DataTxConnAdmin_t*)pArgs;
    gpBle_TxResourceAvailableInd(pContext->connId);
}

void Ble_ReleaseDataTxBuffer(UInt8 BufferId)
{
    Ble_BufferAdmin[BufferId].totalLength = 0;
    Ble_BufferAdmin[BufferId].actualLength = 0;
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    if (Ble_BufferAdmin[BufferId].pConn)
    {
        hciCoreTxAclComplete(Ble_BufferAdmin[BufferId].pConn, Ble_BufferAdmin[BufferId].pWsfBuff);
    }
    Ble_BufferAdmin[BufferId].start = NULL;
    Ble_BufferAdmin[BufferId].pConn = NULL;
#else
    Ble_BufferAdmin[BufferId].start = &Ble_DataTXPool[BufferId][0];
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
    Ble_BufferAdmin[BufferId].cid = BLE_CONN_HANDLE_INVALID;
    Ble_BufferAdmin[BufferId].SequenceNbr = INVALID_SEQ_NBR;
    Ble_BufferAdmin[BufferId].paused = false;
}

UInt8 Ble_DataFindBufferWithSmallestSeqNbr(UInt16 PreviousSmaller)
{
    UIntLoop pool;
    UInt8 BufferWithSmallestSeqNbr = BLE_NUM_TX_DATA_BUFFERS;
    UInt16 SmallestSeqNbr = INVALID_SEQ_NBR;

    for(pool=0; pool< BLE_NUM_TX_DATA_BUFFERS; pool++)
    {
        if ( (Ble_BufferAdmin[pool].cid != BLE_CONN_HANDLE_INVALID) && // valid buffer
             (Ble_BufferAdmin[pool].SequenceNbr > PreviousSmaller) && // ignore buffers with seq nbr smaller than PreviousSmaller: these have been renumbered already
             (Ble_BufferAdmin[pool].SequenceNbr < SmallestSeqNbr)
        )
        {
            SmallestSeqNbr = Ble_BufferAdmin[pool].SequenceNbr;
            BufferWithSmallestSeqNbr = pool;
        }
    }
    return BufferWithSmallestSeqNbr;
}

void Ble_DataHandleSequenceNbrWrap(void)
{
    UIntLoop i;
    UInt8 pool;
    UInt16 SmallestSeqNbr;

    // This function should only be called when Ble_HCI_PacketSeqNbrCount is about to wrap
    // . . . but if you want, it can be invoked at any other point (just change the assert below)
    // Keep in mind: it is not allowed to assign 0 or INVALID_SEQ_NBR to a buffer
    GP_ASSERT_DEV_INT(Ble_HCI_PacketSeqNbrCount == INVALID_SEQ_NBR);

    Ble_HCI_PacketSeqNbrCount = 1;
    SmallestSeqNbr = 0;
    for(i=0;i<BLE_NUM_TX_DATA_BUFFERS; i++)
    {
        pool = Ble_DataFindBufferWithSmallestSeqNbr(SmallestSeqNbr); // there should be at least 1 buffer stored
        if (BLE_NUM_TX_DATA_BUFFERS == pool)
        {
            // no more buffers found: we can stop now
            return;
        }
        else
        {
            SmallestSeqNbr = Ble_BufferAdmin[pool].SequenceNbr;
            Ble_BufferAdmin[pool].SequenceNbr = Ble_HCI_PacketSeqNbrCount; // note: SmallestSeqNbr may equal Ble_HCI_PacketSeqNbrCount: e.g. a buffer that was queued for a long time
            Ble_HCI_PacketSeqNbrCount++;
        }
    }
}

UInt16 Ble_DataCalculateMaxAllowedTxPduSize(Ble_IntConnId_t connectionId, UInt8 CteDuration)
{
    // effectiveOctets are only payload octets
    UInt16 effectiveOctets = gpBle_GetEffectiveMaxTxOctets(connectionId);
    // effectiveTime is time for payload AND overhead (full packet)
    UInt16 effectiveTime = gpBle_GetEffectiveMaxTxTime(connectionId);
    // Max data size
    gpHci_PhyWithCoding_t phyTx = gpBle_GetEffectivePhyTxTypeWithCoding(connectionId);
    // octetsInTime is conversion to actual payload octets  - overhead is taken into account
    UInt16 octetsInTime = gpBleDataCommon_GetOctetsFromDurationUs(effectiveTime, phyTx, CteDuration);

    GP_LOG_PRINTF("eff oct: %u time: %u octInTime %u min: %u",0, effectiveOctets, effectiveTime, octetsInTime, min(effectiveOctets, octetsInTime));

    return min(effectiveOctets, octetsInTime);
}

gpHci_Result_t Ble_DoDataTxRequest(UInt8 pool, gpPd_Loh_t *pdLoh)
{
    Ble_LLID_t      llid;
    UInt16          len;
    UInt16          maxLen;
    gpHci_Result_t  result;
    UInt8           CteDuration = 0;
#ifdef GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX
    gpBleData_CteOptions_t* pCteOptions;
    gpBleData_CteOptions_t localCteOptions;
    pCteOptions = &localCteOptions;
#endif /* GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX */

    if( (Ble_BufferAdmin[pool].actualLength==0) &&
        ( (Ble_BufferAdmin[pool].pb_flag == gpHci_L2CAP_Start) ||
          (Ble_BufferAdmin[pool].pb_flag == gpHci_L2CAP_Start_AutoFlush) ) // BlueZ sets the packet-boundary flag to "2", although this is not allowed by the spec
      )
    {
        GP_LOG_PRINTF("Data Tx: Bf %d: start: Pd %d",0,pool, pdLoh->handle);
        llid = Ble_LLID_DataStart;
    }
    else
    {
        GP_LOG_PRINTF("Data Tx: Bf %d: cont: Pd %d",0,pool, pdLoh->handle);
        llid = Ble_LLID_DataContinuedOrEmpty;
    }

#ifdef GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX
    if (!gpBleDirectionFinding_GetUnsolicitedCteOptions(Ble_BufferAdmin[pool].cid, pCteOptions))
    {
        pCteOptions = NULL;
    }
    else
    {
        CteDuration = pCteOptions->cteDurationUnit * 8;
    }
#endif /* GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX */

    maxLen = Ble_DataCalculateMaxAllowedTxPduSize(Ble_BufferAdmin[pool].cid, CteDuration);
    len = ( (Ble_BufferAdmin[pool].totalLength - Ble_BufferAdmin[pool].actualLength) < maxLen ) ? (Ble_BufferAdmin[pool].totalLength - Ble_BufferAdmin[pool].actualLength) : maxLen;

    pdLoh->length = 0;
    pdLoh->offset = GPBLEDATACOMMON_PDU_FOOTER_MAX_OFFSET;

#ifdef GP_DIVERSITY_DEVELOPMENT
    if(Ble_DataPumpParameters.enabled)
    {
        // If the data pump is running, allow to customize the payload
        Ble_VsdDataPumpInitPayload(Ble_BufferAdmin[pool].start);
    }
#endif

    gpPd_PrependWithUpdate(pdLoh, len, Ble_BufferAdmin[pool].start);


    Ble_BufferAdmin[pool].actualLength += len;
    Ble_BufferAdmin[pool].start +=len;

#ifdef GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX
    result = gpBle_DataTxQueueRequest(Ble_BufferAdmin[pool].cid, *pdLoh, llid, pCteOptions);
#else
    result = gpBle_DataTxQueueRequest(Ble_BufferAdmin[pool].cid, *pdLoh, llid, NULL);
#endif /* GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX */

    if(result != gpHci_ResultSuccess )
    {
        Ble_BufferAdmin[pool].actualLength -= len;
        Ble_BufferAdmin[pool].start -=len;
        Ble_RMFreeResource( Ble_BufferAdmin[pool].cid , pdLoh->handle);
    }
    else
    {
        Ble_ConnAdmin[Ble_BufferAdmin[pool].cid].NumQueuedPds += 1;
        Ble_BufferAdmin[pool].NumFrags += 1;
    }

    // In any case: invalidate the Pd handle
    pdLoh->handle = GP_PD_INVALID_HANDLE;

    return result;
}

UInt8 Ble_DataFindNextBufferForConnection(UInt8 CurrentPool, Ble_IntConnId_t currentCid)
{
    UInt8 pool;
    UInt8 NextBuffer = CurrentPool;
    UInt16 NextBufferSeqNbr;

    NextBufferSeqNbr = (CurrentPool < BLE_NUM_TX_DATA_BUFFERS)? Ble_BufferAdmin[CurrentPool].SequenceNbr : INVALID_SEQ_NBR;
    for(pool=0; pool< BLE_NUM_TX_DATA_BUFFERS; pool++)
    {
        if ( (pool != CurrentPool) &&
             (Ble_BufferAdmin[pool].cid == currentCid) &&
             (Ble_BufferAdmin[pool].SequenceNbr < NextBufferSeqNbr) &&
             (Ble_BufferAdmin[pool].actualLength < Ble_BufferAdmin[pool].totalLength)
        )
        {
            NextBufferSeqNbr = Ble_BufferAdmin[pool].SequenceNbr;
            NextBuffer = pool;
        }
    }
    return NextBuffer;
}

Bool Ble_DataCheckLinkAllowance(Ble_IntConnId_t cid)
{
    UIntLoop i;
    UInt8 TotalNumOccupiedBuffers = 0;
    UInt8 ConnectionStoredBuffers[BLE_LLCP_MAX_NR_OF_CONNECTIONS];
    UInt8 TotalNumQueuedPds = 0;
    UInt8 NumActiveConnections = 0;
    UInt16 HostResourceAllowance;
    UInt16 CurrentAllowance;

    MEMSET(ConnectionStoredBuffers, 0, sizeof(ConnectionStoredBuffers));

    for(i=0; i< BLE_NUM_TX_DATA_BUFFERS; i++)
    {
        if (BLE_CONN_HANDLE_INVALID != Ble_BufferAdmin[i].cid)
        {
            TotalNumOccupiedBuffers++;
            ConnectionStoredBuffers[Ble_BufferAdmin[i].cid]++;
        }
        if ( (cid == Ble_BufferAdmin[i].cid) && (Ble_BufferAdmin[i].paused) )
        {
            return false; // data stransfer on this link has been paused
        }
    }

    GP_LOG_PRINTF("TotNumOcc %d : CStored %d",0, TotalNumOccupiedBuffers, ConnectionStoredBuffers[cid]);
    if (0 == TotalNumOccupiedBuffers)
    {
       // there is no more HCI data to transmit
       return false;
    }

    // Check the Total number of queued Pds
    //  . . . and the number of active connections
    for(i=0; i< BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        TotalNumQueuedPds += Ble_ConnAdmin[i].NumQueuedPds;
        if (ConnectionStoredBuffers[i] > 0)
        {
            NumActiveConnections++;
        }
    }
    GP_LOG_PRINTF("TotNumQd %d : TotNumActCons %d",0, TotalNumQueuedPds, NumActiveConnections);
    if (NumActiveConnections < 2)
    {
       // only 1 connection is active - all resources for 1 link
       return true;
    }
    if (0 == TotalNumQueuedPds)
    {
        // we're about to send the first PDU
        return true;
    }

    GP_ASSERT_DEV_INT(TotalNumOccupiedBuffers > 0);
    GP_ASSERT_DEV_INT(TotalNumQueuedPds > 0);
    HostResourceAllowance = (100*((UInt16)ConnectionStoredBuffers[cid]))/(UInt16)TotalNumOccupiedBuffers;
    CurrentAllowance = (100 * ((UInt16)Ble_ConnAdmin[cid].NumQueuedPds))/(UInt16)TotalNumQueuedPds;
    GP_LOG_PRINTF("HostA %d : CurrentA %d",0, HostResourceAllowance, CurrentAllowance);
    if ( CurrentAllowance >= HostResourceAllowance )
    {
        return false;
    }
    return true;
}

Bool Ble_DataCheckAndGetValidTxResource(gpPd_Loh_t *pdLoh, Ble_IntConnId_t connId)
{
    Bool result = false;

    if (gpPd_ResultValidHandle != gpPd_CheckPdValid(pdLoh->handle))
    {
        pdLoh->handle = gpBle_DataTxQueueAllocatePd(connId, Ble_DataChannelTxQueueCallerData);
    }

    if(gpPd_ResultValidHandle == gpPd_CheckPdValid(pdLoh->handle))
    {
        result = true;
        pdLoh->offset = GPBLEDATACOMMON_PDU_FOOTER_MAX_OFFSET;
        pdLoh->length = 0;
    }
    return result;
}
#ifdef GP_DIVERSITY_DEVELOPMENT
gpHci_Result_t Ble_VsdSetDataPumpParametersChecker(gpHci_VsdSetDataPumpParametersCommand_t* pParams, Bool enableCheck)
{
    if(gpBleLlcp_IsHostConnectionHandleValid(pParams->connHandle) != gpHci_ResultSuccess)
    {
        GP_LOG_SYSTEM_PRINTF("Data pump: invalid connection handle: %x",0,pParams->connHandle);
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    if(pParams->sizeOfPackets == 0 || pParams->sizeOfPackets > GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX)
    {
        GP_LOG_SYSTEM_PRINTF("Invalid packet size: %x < %x <= %x",0,0,pParams->sizeOfPackets, GPBLEDATACOMMON_OCTETS_SUPPORTED_MAX);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    if(pParams->payloadType >= gpHci_VsdDataPumpPayloadTypeInvalid)
    {
        GP_LOG_SYSTEM_PRINTF("Data pump: invalid payload type: %u",0, pParams->payloadType);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    if(enableCheck && Ble_DataPumpParameters.enabled)
    {
        //Ble_DataPumpParameters.enabled = 0;
        GP_LOG_SYSTEM_PRINTF("Not allowed to change data pump params while enabled",0);
        return gpHci_ResultCommandDisallowed;
    }

    return gpHci_ResultSuccess;
}

gpHci_Result_t Ble_VsdSetDataPumpEnableChecker(Bool enable)
{
    if(Ble_DataPumpParameters.enabled && enable)
    {
        GP_LOG_SYSTEM_PRINTF("Data pump already enabled",0);
        return gpHci_ResultCommandDisallowed;
    }

    if(!Ble_DataPumpParameters.enabled && !enable)
    {
        GP_LOG_SYSTEM_PRINTF("Data pump already disabled",0);
        return gpHci_ResultCommandDisallowed;
    }

    return gpHci_ResultSuccess;
}

gpHci_Result_t Ble_VsdSetDataPumpEnableAction(Bool enable)
{
    gpHci_Result_t result;

    result = Ble_VsdSetDataPumpParametersChecker(&Ble_DataPumpParameters.vsdParams, false);

    if(result != gpHci_ResultSuccess)
    {
        return result;
    }
    else
    {
        Ble_DataPumpParameters.enabled = enable;

        if(enable)
        {
            Ble_DataPumpParameters.confirmPacketCount = 0;
            Ble_DataPumpParameters.currentpacketCount = 0;
            Ble_DataPumpParameters.incrementalPerPacketStartByte = 0;
            Ble_VsdDataPumpTriggerNextTransmission();
        }
    }

    return result;
}

void Ble_VsdDataPumpTriggerNextTransmission(void)
{
    UIntLoop i;
    UInt8 nrOfBuffers = 0;
    UInt8 nrOfAddedPackets = 0;
    UInt8 numFreeBuffers = 0;
    UInt8 pool;
    UInt16 numTodo;

    // Find free buffers
    for(pool=0; pool < BLE_NUM_TX_DATA_BUFFERS; pool++)
    {
        if (BLE_CONN_HANDLE_INVALID == Ble_BufferAdmin[pool].cid)
        {
            ++numFreeBuffers;
        }
    }

    if(Ble_DataPumpParameters.vsdParams.nrOfPackets == 0)
    {
        // Unlimited: use max amount of buffers
        numTodo = BLE_NUM_TX_DATA_BUFFERS;
    }
    else
    {
        numTodo = Ble_DataPumpParameters.vsdParams.nrOfPackets - Ble_DataPumpParameters.currentpacketCount;
    }

    nrOfBuffers = (numTodo <  numFreeBuffers) ? numTodo : numFreeBuffers;

    for(i = 0; i < nrOfBuffers; i++)
    {
#ifndef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
        gpBle_DataTxRequest(Ble_DataPumpParameters.vsdParams.connHandle, Ble_DataPumpParameters.vsdParams.sizeOfPackets, BleDataTx_VsdPacketData);
#else
        gpBle_DataTxRequest(Ble_DataPumpParameters.vsdParams.connHandle, Ble_DataPumpParameters.vsdParams.sizeOfPackets, BleDataTx_VsdPacketData,NULL,NULL);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */

        nrOfAddedPackets++;
    }

    Ble_DataPumpParameters.currentpacketCount += nrOfAddedPackets;
}

void Ble_VsdDataPumpInitPayload(UInt8* pPayload)
{
    switch(Ble_DataPumpParameters.vsdParams.payloadType)
    {
        case gpHci_VsdDataPumpPayloadTypeFixed:
        {
            MEMSET(&pPayload[0], Ble_DataPumpParameters.vsdParams.payloadStartByte, Ble_DataPumpParameters.vsdParams.sizeOfPackets);
            break;
        }
        case gpHci_VsdDataPumpPayloadTypeIncrementalInPacket:
        {
            // Incremental
            UIntLoop i;

            for(i = 0; i < Ble_DataPumpParameters.vsdParams.sizeOfPackets; i++)
            {
                pPayload[i] = Ble_DataPumpParameters.vsdParams.payloadStartByte++;
            }
            break;
        }
        case gpHci_VsdDataPumpPayloadTypeIncrementalPerPacket:
        {
            UIntLoop i;

            // First byte is a round identifier (will remain constant for 255 packets)
            pPayload[0] = Ble_DataPumpParameters.incrementalPerPacketStartByte;

            for(i = 1; i < Ble_DataPumpParameters.vsdParams.sizeOfPackets; i++)
            {
                pPayload[i] = Ble_DataPumpParameters.vsdParams.payloadStartByte;
            }

            if(Ble_DataPumpParameters.vsdParams.payloadStartByte == 255)
            {
                Ble_DataPumpParameters.incrementalPerPacketStartByte += 1;
            }

            Ble_DataPumpParameters.vsdParams.payloadStartByte++;
            break;
        }
        case gpHci_VsdDataPumpPayloadTypeRandom:
        {
            // Random
            gpRandom_GetNewSequence(Ble_DataPumpParameters.vsdParams.sizeOfPackets, &pPayload[0]);
            break;
        }
        default:
        {
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }
}

#endif //GP_DIVERSITY_DEVELOPMENT

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBle_DataTxInit(void)
{
#ifdef GP_DIVERSITY_DEVELOPMENT
    // Fill with pattern
    MEMSET(BleDataTx_VsdPacketData, 0xAA, sizeof(BleDataTx_VsdPacketData));
    MEMSET(&Ble_DataPumpParameters, 0, sizeof(Ble_DataPumpParameters));
    Ble_DataPumpParameters.vsdParams.connHandle = GP_HCI_CONNECTION_HANDLE_INVALID;
    Ble_DataPumpParameters.enabled = 0;
#endif
}

void gpBle_DataTxReset(Bool firstReset)
{
    NOT_USED(firstReset);
    UIntLoop i;

    GP_LOG_PRINTF("Data TX Reset",0);
    Ble_activeCid = BLE_CONN_HANDLE_INVALID;
    Ble_HCI_PacketSeqNbrCount = 1; // never assign value 0 - simplify seq nbr wrap handling
    for(i=0; i< BLE_NUM_TX_DATA_BUFFERS; i++)
    {
        Ble_ReleaseDataTxBuffer(i);
    }

    for(i=0; i< BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        Ble_ConnAdmin[i].connId = Ble_IntConnId_Invalid;
        Ble_ConnAdmin[i].NumQueuedPds = 0;
        Ble_ConnAdmin[i].pauseCounter = 0;
    }

#ifdef GP_DIVERSITY_DEVELOPMENT
    MEMSET(&Ble_DataPumpParameters, 0, sizeof(Ble_DataPumpParameters));
    Ble_DataPumpParameters.vsdParams.connHandle = GP_HCI_CONNECTION_HANDLE_INVALID;
    Ble_DataPumpParameters.enabled = 0;
#endif //GP_DIVERSITY_DEVELOPMENT
}

void gpBleDataTx_cbDataConfirm(Ble_IntConnId_t connId)
{
    UIntLoop i;
    UInt8 NextBuffer;

    Ble_activeCid = connId;
    GP_ASSERT_DEV_INT(Ble_ConnAdmin[connId].NumQueuedPds>0);
    Ble_ConnAdmin[connId].NumQueuedPds -= 1;

    GP_LOG_PRINTF("TxD conf actCid%d ",0, Ble_activeCid);

    // Now check if data conf is for the last fragment of an HCI frame . . . if so, then send a HCI NOCP event
    // Case analysis:
    // 1) There is a HCI buffer for this cid with actualLength == totalLength
    //    Note there could be multiple such buffers - fortunately we can cheat a little: the "order" of NOCP events is irrelevant
    //    So simply decrement the NumFrags until it reaches 0
    // 2) The conf is for a buffer that is still being fragmented : do not send the HCI NOCP event yet
    //    Such buffer can be found with Ble_DataFindNextBufferForConnection - and there is only 1 such buffer
    //    Check if the current buffer is real and if so decrement the NumFrags
    for( i=0; i < BLE_NUM_TX_DATA_BUFFERS; i++)
    {
        if ( (Ble_activeCid == Ble_BufferAdmin[i].cid) &&
             (BLE_CONN_HANDLE_INVALID != Ble_BufferAdmin[i].cid) &&
             (Ble_BufferAdmin[i].totalLength) &&
             (Ble_BufferAdmin[i].actualLength == Ble_BufferAdmin[i].totalLength)
           )
        {
           Ble_BufferAdmin[i].NumFrags -= 1;
           GP_LOG_PRINTF("TxD conf: Bf %d Nf %d fin",0,i,Ble_BufferAdmin[i].NumFrags);
           if (0 == Ble_BufferAdmin[i].NumFrags)
           {
#ifdef GP_DIVERSITY_DEVELOPMENT
             Ble_IntConnId_t DataPumpCid;
#endif //GP_DIVERSITY_DEVELOPMENT

             gpHci_ConnectionHandle_t connHandle = Ble_BufferAdmin[i].connHandle;

             Ble_ReleaseDataTxBuffer(i);
#ifdef GP_DIVERSITY_DEVELOPMENT
             DataPumpCid = gpBleLlcp_HciHandleToIntHandle(BLE_CONN_HANDLE_HANDLE_GET(Ble_DataPumpParameters.vsdParams.connHandle));
             if (connId == DataPumpCid && Ble_DataPumpParameters.enabled)
             {
               Ble_DataPumpParameters.confirmPacketCount +=1;
               if(Ble_DataPumpParameters.currentpacketCount == Ble_DataPumpParameters.confirmPacketCount)
               {
                 GP_LOG_PRINTF("Data pump packet count reached, stop",0);
                 // Nr of wanted packet reached ==> stop
                 Ble_DataPumpParameters.enabled = false;
               }
               else
               {
                 //gpSched_ScheduleEvent(0,Ble_VsdDataPumpTriggerNextTransmission); // with this, we cannot fill our connection events with more than 7 packets (when using short data pkts)
                 Ble_VsdDataPumpTriggerNextTransmission();
               }
             }
             else
#endif //GP_DIVERSITY_DEVELOPMENT
             {
               //  GP_LOG_SYSTEM_PRINTF("NOCP >> cid %d pump CID %d enabled  %d ",2, connId, DataPumpCid, Ble_DataPumpParameters.enabled);
               gpBle_SendHciNumberOfCompletedPacketsEvent(connHandle);
             }
           }
           return;
        }
    }

    NextBuffer = Ble_DataFindNextBufferForConnection(BLE_NUM_TX_DATA_BUFFERS, connId);
    if (BLE_NUM_TX_DATA_BUFFERS != NextBuffer)
    {
      Ble_BufferAdmin[NextBuffer].NumFrags -= 1;
      GP_LOG_PRINTF("TxD conf: Bf %d Nf %d rem %d",0,NextBuffer,Ble_BufferAdmin[NextBuffer].NumFrags, Ble_BufferAdmin[NextBuffer].totalLength - Ble_BufferAdmin[NextBuffer].actualLength);
    }
}

void gpBle_TxResourceAvailableInd(Ble_IntConnId_t connId)
{
    gpPd_Loh_t  pdLoh;
    UInt8       pool;
    UInt8       NextBuffer;

    pdLoh.handle = GP_PD_INVALID_HANDLE;
    // We first try sending data on the active connection
    // ... use the available Tx resources for the connection with an ongoing connection event
    if (connId != BLE_CONN_HANDLE_INVALID)
    {
        while (Ble_DataCheckLinkAllowance(connId))
        {
            NextBuffer = Ble_DataFindNextBufferForConnection(BLE_NUM_TX_DATA_BUFFERS, connId);
            if ( (NextBuffer >= BLE_NUM_TX_DATA_BUFFERS) ||
                 (!Ble_DataCheckAndGetValidTxResource(&pdLoh, connId)) ||
                 (gpHci_ResultSuccess != Ble_DoDataTxRequest(NextBuffer, &pdLoh ))
               )
            {
              break; // try another connection
            }
        }
    }

    GP_LOG_PRINTF("Data Tx - TRY send data - check Link Allowance on other links",0);
    // Find another link with data to transmit
    // Take care: Ble_activePool and Ble_activeCid could be invalid
    for(pool=0; pool< BLE_NUM_TX_DATA_BUFFERS; pool++)
    {
        if ( (Ble_BufferAdmin[pool].actualLength < Ble_BufferAdmin[pool].totalLength) &&
             (Ble_activeCid != Ble_BufferAdmin[pool].cid) // even if Ble_activeCid == BLE_CONN_HANDLE_INVALID
        )
        while(Ble_DataCheckLinkAllowance(Ble_BufferAdmin[pool].cid))
        {
            // We found a link with data and with "good allowance" - transmit the first buffer of that link
            NextBuffer = Ble_DataFindNextBufferForConnection(BLE_NUM_TX_DATA_BUFFERS, Ble_BufferAdmin[pool].cid);
            if ( (NextBuffer >= BLE_NUM_TX_DATA_BUFFERS) ||
                 (!Ble_DataCheckAndGetValidTxResource(&pdLoh, Ble_BufferAdmin[pool].cid)) ||
                 (gpHci_ResultSuccess != Ble_DoDataTxRequest(NextBuffer, &pdLoh ))
            )
            {
                break; // try another buffer or another connection
            }
        }
    }
}

void gpBle_DataTxOpenConnection(Ble_IntConnId_t connId)
{
    GP_LOG_PRINTF("Data Tx: open conn",0);

    Ble_ConnAdmin[connId].connId = connId;
    Ble_ConnAdmin[connId].NumQueuedPds = 0;
    Ble_ConnAdmin[connId].pauseCounter = 0;
}

void gpBle_DataTxSetConnectionPause(Ble_IntConnId_t connId, Bool pause)
{
    UIntLoop pool;

    GP_LOG_PRINTF("Data Tx: set pause conn: %x",0,pause);

    if(Ble_ConnAdmin[connId].connId == Ble_IntConnId_Invalid)
    {
        // Ignore pause requests on connections that are closed
        return;
    }

    if(pause)
    {
        GP_ASSERT_DEV_INT(Ble_ConnAdmin[connId].pauseCounter != 0xFF);
        Ble_ConnAdmin[connId].pauseCounter++;
    }
    else
    {
        GP_ASSERT_DEV_INT(Ble_ConnAdmin[connId].pauseCounter > 0);
        Ble_ConnAdmin[connId].pauseCounter--;
    }

    for( pool=0; pool < BLE_NUM_TX_DATA_BUFFERS; pool++)
    {
        if(connId == Ble_BufferAdmin[pool].cid)
        {
            Ble_BufferAdmin[pool].paused = Ble_ConnAdmin[connId].pauseCounter > 0;
        }
    }

    if(Ble_ConnAdmin[connId].pauseCounter == 0)
    {
        // In case the link is unpaused, we need to provide an artifical trigger to resume data for that link
        gpSched_ScheduleEventArg(0, Ble_DataTxUnpauseFollowup, &Ble_ConnAdmin[connId]);
    }
}

void gpBle_DataTxCloseConnection(Ble_IntConnId_t connId)
{
    UIntLoop    pool;

#ifdef GP_DIVERSITY_DEVELOPMENT
    Ble_IntConnId_t DataPumpCid;

    DataPumpCid = gpBleLlcp_HciHandleToIntHandle(BLE_CONN_HANDLE_HANDLE_GET(Ble_DataPumpParameters.vsdParams.connHandle));
#endif //GP_DIVERSITY_DEVELOPMENT

    GP_LOG_PRINTF("Data Tx: close conn",0);

    for( pool=0; pool < BLE_NUM_TX_DATA_BUFFERS; pool++)
    {
        if(connId == Ble_BufferAdmin[pool].cid)
        {
            // do not send HCI NoCP evt: cfr BT v4.2 Vol2 Part E $4.3
            Ble_ReleaseDataTxBuffer(pool);
        }
    }

#ifdef GP_DIVERSITY_DEVELOPMENT
    if (DataPumpCid == connId && Ble_DataPumpParameters.enabled)
    {
        Ble_DataPumpParameters.enabled = false;
    }
#endif //GP_DIVERSITY_DEVELOPMENT

    if (Ble_activeCid == connId)
    {
      Ble_activeCid = BLE_CONN_HANDLE_INVALID;
    }
    Ble_ConnAdmin[connId].NumQueuedPds = 0;
    Ble_ConnAdmin[connId].pauseCounter = 0;
    Ble_ConnAdmin[connId].connId = Ble_IntConnId_Invalid;
}

#if !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY))
void gpBle_DataTxRequest(gpHci_ConnectionHandle_t connHandle, UInt16 dataLength, UInt8* pData)
#else
void gpBle_DataTxRequest(gpHci_ConnectionHandle_t connHandle, UInt16 dataLength, UInt8* pData, void* pConn, void* pWsfBuff)
#endif //!(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY))
{
    gpPd_Loh_t          pdLoh;
    UIntLoop            pool;
    Ble_IntConnId_t     cid;

    GP_LOG_PRINTF("Received data TX request for conn: %x (len: %x)",0,connHandle, dataLength);

    cid = gpBleLlcp_HciHandleToIntHandle(BLE_CONN_HANDLE_HANDLE_GET(connHandle));
    if ( (BLE_CONN_HANDLE_INVALID == cid) ||
         (!gpBle_IsConnectionOpen(cid))
    )
    {
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
        if (pConn)
        {
            hciCoreTxAclComplete(pConn, pWsfBuff);
        }
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
        gpBle_SendHciNumberOfCompletedPacketsEvent(BLE_CONN_HANDLE_HANDLE_GET(connHandle));
        return;
    }

    // Find a free buffer
    for(pool=0; pool < BLE_NUM_TX_DATA_BUFFERS; pool++)
    {
        if (BLE_CONN_HANDLE_INVALID == Ble_BufferAdmin[pool].cid)
        {
            break;
        }
    }
    if (pool >= BLE_NUM_TX_DATA_BUFFERS)
    {
        GP_LOG_SYSTEM_PRINTF("ERROR NO ROOM IN TX POOL",0);
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
        if (pConn)
        {
            hciCoreTxAclComplete(pConn, pWsfBuff);
        }
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */

        // Note that we can send a HCI Data Buffer Overflow event on an empty HCI frame . . . which is weird but acceptable
        Ble_SendDataBufferOverflowEvent(gpHci_LinkType_Asynchronous);
        return;
    }
    if (0 == dataLength) // known limitation: no HCI Empty data (even if it is a continuation fragment sent by the Host)
    {
        /*
         * Note: If the Link Layer receives an HCI ACL Data Packet with
         * Data_Total_Length equal to 00000000b and Packet_Boundary_Flag set to 10b
         * (i.e., a start fragment), then the Link Layer cannot simply transmit the fragment
         * over the air but, instead, must combine it with one of more of the following
         * continuation fragments to form a PDU with LLID set to 10b and non-zero length.
         */
        // an empty continuation must not overwrite the PREVIOUS_WAS_EMPTYSTART condition
        if (gpHci_L2CAP_Start == BLE_CONN_HANDLE_PB_GET(connHandle))
        {
            Ble_BufferAdmin[pool].pb_flag = BLE_PB_FLAG_PREVIOUS_WAS_EMPTYSTART;
        }
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
        if (pConn)
        {
            hciCoreTxAclComplete(pConn, pWsfBuff);
        }
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
        gpBle_SendHciNumberOfCompletedPacketsEvent(BLE_CONN_HANDLE_HANDLE_GET(connHandle));
        return;
    }
    // Allocate the buffer to the new HCI frame
    GP_LOG_PRINTF("Store HCI Frame in Bf %d",0,pool);
#ifdef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    Ble_BufferAdmin[pool].start = pData;
    Ble_BufferAdmin[pool].pConn = pConn;
    Ble_BufferAdmin[pool].pWsfBuff = pWsfBuff;
#else
    MEMCPY( Ble_BufferAdmin[pool].start, pData, dataLength );
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
    Ble_BufferAdmin[pool].totalLength = dataLength;
    Ble_BufferAdmin[pool].connHandle = BLE_CONN_HANDLE_HANDLE_GET(connHandle);
    if ( Ble_BufferAdmin[pool].pb_flag == BLE_PB_FLAG_PREVIOUS_WAS_EMPTYSTART )
    {
        Ble_BufferAdmin[pool].pb_flag = gpHci_L2CAP_Start;
    }
    else
    {
        Ble_BufferAdmin[pool].pb_flag = BLE_CONN_HANDLE_PB_GET(connHandle);
    }
    Ble_BufferAdmin[pool].cid = cid;
    Ble_BufferAdmin[pool].NumFrags = 0;
    Ble_BufferAdmin[pool].SequenceNbr = Ble_HCI_PacketSeqNbrCount;
    Ble_BufferAdmin[pool].paused = Ble_ConnAdmin[cid].pauseCounter > 0;

    // Increment our Packet Sequence Counter: we use it for in-order handling of HCI buffers and fragments
    Ble_HCI_PacketSeqNbrCount++;
    if (INVALID_SEQ_NBR == Ble_HCI_PacketSeqNbrCount)
    {
        Ble_DataHandleSequenceNbrWrap();
    }

    // Check if a transmission (from another buffer) is already ongoing for this connection
    // If so, do not start transmitting the new buffer - but wait until the previous one has been completed
    {
        UInt8 NextBuffer = Ble_DataFindNextBufferForConnection(pool, Ble_BufferAdmin[pool].cid);
        if (NextBuffer != pool)
        {
            // another buffer is being transmitted for this connection
            return;
        }
    }

    // Start transmitting from the new buffer
    while (Ble_BufferAdmin[pool].actualLength < Ble_BufferAdmin[pool].totalLength)
    {
        GP_LOG_PRINTF("Data Req: aL=%d tL=%d",0,Ble_BufferAdmin[pool].actualLength,Ble_BufferAdmin[pool].totalLength);
        if (!Ble_DataCheckLinkAllowance(Ble_BufferAdmin[pool].cid))
        {
            // The link is currently not allowed to transmit any more (most probably a few PDUs were queued already by this loop)
            return;
        }

        pdLoh.handle = gpBle_DataTxQueueAllocatePd(Ble_BufferAdmin[pool].cid, Ble_DataChannelTxQueueCallerData);
        if(gpPd_ResultValidHandle != gpPd_CheckPdValid(pdLoh.handle))
        {
            GP_LOG_PRINTF("No room for TX, wait! len %d ",2, Ble_BufferAdmin[pool].actualLength );
            return;
        }
        if(gpHci_ResultSuccess != Ble_DoDataTxRequest(pool, &pdLoh))
        {
            return;
        }
    }
}

Bool gpBle_DataTxIsDataInBuffers(Ble_IntConnId_t connId)
{
    UInt8 NextBuffer;
    NextBuffer = Ble_DataFindNextBufferForConnection(BLE_NUM_TX_DATA_BUFFERS, connId);
    return (BLE_NUM_TX_DATA_BUFFERS != NextBuffer);
}

/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_LeReadBufferSize(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_LEReadBufferSize_t    leReadBuffersize;

    // Following are constant values - they never change in run-time
    leReadBuffersize.ACLDataPacketLength = GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_TX;
    leReadBuffersize.totalNumDataPackets = BLE_NUM_TX_DATA_BUFFERS;

    GP_LOG_PRINTF("gpBle_ReadBufferSize",0);
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    MEMCPY(&pEventBuf->payload.commandCompleteParams.returnParams.leReadBufferSize, &leReadBuffersize, sizeof(gpHci_LEReadBufferSize_t));

    return gpHci_ResultSuccess;
}

#ifdef GP_DIVERSITY_DEVELOPMENT
gpHci_Result_t gpBle_VsdSetDataPumpParameters(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_VsdSetDataPumpParametersCommand_t* pDataPumpParams = &pParams->VsdSetDataPumpParameters;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = Ble_VsdSetDataPumpParametersChecker(pDataPumpParams, true);

    if(result == gpHci_ResultSuccess)
    {
        MEMCPY(&Ble_DataPumpParameters.vsdParams, pDataPumpParams, sizeof(gpHci_VsdSetDataPumpParametersCommand_t));
    }

    return result;
}

gpHci_Result_t gpBle_VsdSetDataPumpEnable(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = Ble_VsdSetDataPumpEnableChecker(pParams->VsdSetDataPumpEnable.enable);

    if(result == gpHci_ResultSuccess)
    {
        GP_LOG_PRINTF("Enable data pump: %x",0,pParams->VsdSetDataPumpEnable.enable);
        result = Ble_VsdSetDataPumpEnableAction(pParams->VsdSetDataPumpEnable.enable);
    }

    return result;
}
#endif //GP_DIVERSITY_DEVELOPMENT
