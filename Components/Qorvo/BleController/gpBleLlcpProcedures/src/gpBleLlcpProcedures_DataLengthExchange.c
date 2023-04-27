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
#include "gpBleDataTx.h"
#include "gpBleDataChannelTxQueue.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcpProcedures_defs.h"
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpSched.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_LLCP_DATA_LENGTH_PDU_NR_OF_FIELDS       4

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// Keep the latest data length change (avoid duplicates when local and remote are simultaneously issuing length updates)
static gpHci_LeMetaDataLengthChange_t Ble_LlcpLastDataLengthChange;

/*****************************************************************************
 *                    Static Function Prototypes
*****************************************************************************/

static Ble_LlcpFrameworkAction_t Ble_LlcpDataLengthStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpDataLengthGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* ctrDataLength, UInt8* pCtrData);
static Ble_LlcpFrameworkAction_t Ble_LlcpDataLengthStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpDataLengthPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpDataLengthPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static void Ble_LlcpDataLengthFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpRegisterDataLengthExchangeProcedure(void);

static void Ble_TriggerCbDataLengthChanged(gpHci_LeMetaDataLengthChange_t* pParams);

/*****************************************************************************
 *                    Procedure descriptor
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_DataLengthExchangePduDescriptors[] =
{
    {gpBleLlcp_OpcodeLengthReq, 8, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH},
    {gpBleLlcp_OpcodeLengthRsp, 8, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH}
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_DataLengthExchangeDescriptor =
{
    .procedureFlags = 0x00,
    .procedureDataLength = sizeof(Ble_LengthUpdateData_t),
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_DataLengthExchangePduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_DataLengthExchangePduDescriptors,
    .featureMask = BM(gpBleConfig_FeatureIdDataPacketLengthExtension),
    .cbQueueingNeeded = NULL,
    .cbProcedureStart = Ble_LlcpDataLengthStart,
    .cbGetCtrData = Ble_LlcpDataLengthGetCtrData,
    .cbStoreCtrData = Ble_LlcpDataLengthStoreCtrData,
    .cbPduReceived = Ble_LlcpDataLengthPduReceived,
    .cbUnexpectedPduReceived = Ble_LlcpCommonUnexpectedPduReceived,
    .cbPduQueued = Ble_LlcpDataLengthPduQueued,
    .cbPduTransmitted = NULL,
    .cbFinished = Ble_LlcpDataLengthFinished
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Ble_LlcpFrameworkAction_t Ble_LlcpDataLengthStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    Ble_LengthUpdateData_t* pLengthUpdateData = (Ble_LengthUpdateData_t*)pProcedure->pData;

    GP_ASSERT_DEV_INT(pProcedure);

    if(!pProcedure->localInit)
    {
        Ble_LlcpProcedureContext_t* pPurgedProcedure;

        pPurgedProcedure = gpBleLlcpFramework_PurgeFirstQueuedProcedure(pContext->hciHandle, pProcedure->procedureId);

        if(pPurgedProcedure == NULL)
        {
            // No length update found, just continue
            pLengthUpdateData->localTxOctets = gpBle_GetMaxTxOctetsLocal(pContext->connId);
            pLengthUpdateData->localTxTime = gpBle_GetMaxTxTimeLocal(pContext->connId);
        }
        else
        {
            Ble_LengthUpdateData_t* pPurgedUpdateData;

            GP_LOG_PRINTF("Purged length update procedure --> merge",0);

            pPurgedUpdateData = (Ble_LengthUpdateData_t*)pPurgedProcedure->pData;
            // Length update found, use procedure data
            pLengthUpdateData->localTxOctets = pPurgedUpdateData->localTxOctets;
            pLengthUpdateData->localTxTime = pPurgedUpdateData->localTxTime;

            gpBle_DataTxSetConnectionPause(pContext->connId, true);
        }
    }

    // Store the effectives (needed for comparision later)
    pLengthUpdateData->effectiveTxOctets = gpBle_GetEffectiveMaxTxOctets(pContext->connId);
    pLengthUpdateData->effectiveTxTime = gpBle_GetEffectiveMaxTxTime(pContext->connId);
    pLengthUpdateData->effectiveRxOctets = gpBle_GetEffectiveMaxRxOctets(pContext->connId);
    pLengthUpdateData->effectiveRxTime = gpBle_GetEffectiveMaxRxTime(pContext->connId);


    if(pProcedure->localInit)
    {
        gpBle_DataTxSetConnectionPause(pContext->connId, true);
        return Ble_LlcpFrameworkActionWaitForEmptyTxQueue;
    }
    else
    {
        return Ble_LlcpFrameworkActionContinue;
    }
}

void Ble_LlcpDataLengthGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* ctrDataLength, UInt8* pCtrData)
{
    Ble_LengthUpdateData_t* pLengthUpdateData;
    UInt16 localRxOctets;
    UInt16 localRxTime;

    GP_ASSERT_DEV_INT(pProcedure);

    localRxOctets = gpBle_GetMaxRxOctetsLocal(pContext->connId);
    localRxTime = gpBle_GetMaxRxTimeLocal(pContext->connId);


    pLengthUpdateData = (Ble_LengthUpdateData_t*)pProcedure->pData;

    GP_ASSERT_DEV_INT(pLengthUpdateData);

    switch(*pOpcode)
    {
        case gpBleLlcp_OpcodeLengthReq:
        case gpBleLlcp_OpcodeLengthRsp:
        {
            UIntLoop i;
            UInt16 values[BLE_LLCP_DATA_LENGTH_PDU_NR_OF_FIELDS] =
            {
                localRxOctets,
                localRxTime,
                pLengthUpdateData->localTxOctets,
                pLengthUpdateData->localTxTime
            };

            *ctrDataLength = 0;

            for(i = 0; i < BLE_LLCP_DATA_LENGTH_PDU_NR_OF_FIELDS; i++)
            {
                MEMCPY(&pCtrData[*ctrDataLength], (UInt8*)&values[i], 2);
                *ctrDataLength += 2;
            }
            break;
        }
        default:
        {
            // Should not happen
            GP_ASSERT_DEV_INT(false);
        }
    }
}

Ble_LlcpFrameworkAction_t Ble_LlcpDataLengthStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    switch(opcode)
    {
        case gpBleLlcp_OpcodeLengthReq:
        case gpBleLlcp_OpcodeLengthRsp:
        {
            Ble_LengthUpdateData_t* pLengthUpdateData;
            UInt16 rxOctetsRemote;
            UInt16 rxTimeRemote;
            UInt16 txOctetsRemote;
            UInt16 txTimeRemote;

            pLengthUpdateData = (Ble_LengthUpdateData_t*)pProcedure->pData;
            GP_ASSERT_DEV_INT(pLengthUpdateData);

            gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&rxOctetsRemote);
            gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&rxTimeRemote);
            gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&txOctetsRemote);
            gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&txTimeRemote);

            if(!gpBle_DataOctetsAndTimesValidMinimum(rxOctetsRemote, rxTimeRemote, txOctetsRemote, txTimeRemote))
            {
                // Only check minimum, max can be extended by future revisions of the spec
                pProcedure->result = gpHci_ResultInvalidLMPParametersInvalidLLParameters;
                return Ble_LlcpFrameworkActionRejectWithUnknownRsp;
            }

            gpBle_SetMaxRxOctetsRemote(pContext->connId, rxOctetsRemote);
            gpBle_SetMaxRxTimeRemote(pContext->connId, rxTimeRemote);
            gpBle_SetMaxTxOctetsRemote(pContext->connId,txOctetsRemote);
            gpBle_SetMaxTxTimeRemote(pContext->connId, txTimeRemote);

            break;
        }
        default:
        {
            GP_ASSERT_DEV_INT(false);   // Should not happen
            break;
        }
    }

    return Ble_LlcpFrameworkActionContinue;
}

Ble_LlcpFrameworkAction_t Ble_LlcpDataLengthPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodeLengthReq:
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeLengthRsp;
            break;
        }
        case gpBleLlcp_OpcodeLengthRsp:
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;
            action = Ble_LlcpFrameworkActionStop;
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

Ble_LlcpFrameworkAction_t Ble_LlcpDataLengthPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;
    Ble_LengthUpdateData_t* pLengthUpdateData;

    pLengthUpdateData = (Ble_LengthUpdateData_t*)pProcedure->pData;
    GP_ASSERT_DEV_INT(pLengthUpdateData);

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodeLengthReq:
        {
            // Pause tx data and wait for the empty queue
            gpBle_DataTxSetConnectionPause(pContext->connId, true);

            pProcedure->currentTxPdu = gpBleLlcp_OpcodeLengthRsp;

            action = Ble_LlcpFrameworkActionWaitForEmptyTxQueue;
            break;
        }
        case gpBleLlcp_OpcodeLengthRsp:
        {
            action = Ble_LlcpFrameworkActionStop;
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

void Ble_LlcpDataLengthFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    gpHci_LeMetaDataLengthChange_t params;
    Ble_LengthUpdateData_t* pPrevEffectiveMaxParams;

    GP_ASSERT_DEV_INT(pProcedure != NULL);

    if(pProcedure->currentTxPdu == gpBleLlcp_OpcodeInvalid && !gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_PURGED_IDX))
    {
        // Procedure was queued, but no packets were exchanged (and procedure was not purged), return immediately
        return;
    }

    if(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX) && !pProcedure->localInit)
    {
        // A remote-initiated procedure was rejected => do nothing
        return;
    }

    if(pContext->terminationOngoing)
    {
        // Do nothing when we are in the process of terminating the link
        return;
    }

    gpBle_DataTxSetConnectionPause(pContext->connId, false);

    // After having applied the new length settings (locally): re-calculate bandwidth control parameters.
    // Note that the remote device may not apply the new settings immediately (e.g. it may still send some longer packets than now agreed)
    // Worst case, this will cause a few connection events to be skipped
    // . . . in extreme conditions (lots of too-long packets and a very short LSTO), this may cause a dropped link
    // It is very difficult to do detect the remote's behavior: no explicit LLCP signalling (see also BT SIG Errata 7209 and 7331)
    // One possibility is to monitor the actual length of data packets received after the length negotiation
    // and only change the guard after the peer device has applied the new max length
    gpBle_SetConnectionBandwidthControl(pContext->connId, pContext->ccParams.maxCELength, pContext->intervalUnit);

    pPrevEffectiveMaxParams = (Ble_LengthUpdateData_t*)pProcedure->pData;

    gpBle_SetMaxTxOctetsLocal(pContext->connId, pPrevEffectiveMaxParams->localTxOctets);
    gpBle_SetMaxTxTimeLocal(pContext->connId, pPrevEffectiveMaxParams->localTxTime);

    if(pProcedure->result == gpHci_ResultDifferentTransactionCollision || !notifyHost)
    {
        return;
    }

    params.connectionHandle = pContext->hciHandle;
    params.MaxTxOctets = gpBle_GetEffectiveMaxTxOctets(pContext->connId);
    params.MaxTxTime = gpBle_GetEffectiveMaxTxTime(pContext->connId);
    params.MaxRxOctets = gpBle_GetEffectiveMaxRxOctets(pContext->connId);
    params.MaxRxTime = gpBle_GetEffectiveMaxRxTime(pContext->connId);


    // Send HCI LE Meta event to host if any of the connEffectiveMax parameters changed
    if( pPrevEffectiveMaxParams->effectiveTxOctets != params.MaxTxOctets ||
        pPrevEffectiveMaxParams->effectiveTxTime != params.MaxTxTime     ||
        pPrevEffectiveMaxParams->effectiveRxOctets != params.MaxRxOctets ||
        pPrevEffectiveMaxParams->effectiveRxTime != params.MaxRxTime
      )
    {

        if(MEMCMP(&Ble_LlcpLastDataLengthChange, &params, sizeof(gpHci_LeMetaDataLengthChange_t)) != 0)
        {
            // Only trigger length update if different from the previous
            MEMCPY(&Ble_LlcpLastDataLengthChange, &params, sizeof(gpHci_LeMetaDataLengthChange_t));
            Ble_TriggerCbDataLengthChanged(&params);

            GP_LOG_PRINTF("DL change",0);
            GP_LOG_PRINTF("prev: %i %i %i %i",0,pPrevEffectiveMaxParams->effectiveTxOctets, pPrevEffectiveMaxParams->effectiveTxTime, pPrevEffectiveMaxParams->effectiveRxOctets, pPrevEffectiveMaxParams->effectiveRxTime);
            GP_LOG_PRINTF("curr: %i %i %i %i",0,params.MaxTxOctets, params.MaxTxTime, params.MaxRxOctets, params.MaxRxTime);
        }
    }
}

void Ble_LlcpRegisterDataLengthExchangeProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdDataLengthUpdate, &BleLlcpProcedures_DataLengthExchangeDescriptor);
}

void Ble_TriggerCbDataLengthChanged(gpHci_LeMetaDataLengthChange_t* pParams)
{
    gpBle_EventBuffer_t* pEventBuf;

    pEventBuf = gpBle_AllocateEventBuffer(Ble_EventBufferType_Unsolicited);
    GP_ASSERT_DEV_EXT(pEventBuf != NULL);

    pEventBuf->eventCode = gpHci_EventCode_LEMeta;
    pEventBuf->payload.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeDataLengthChange;
    MEMCPY(&pEventBuf->payload.metaEventParams.params.dataLengthChange, pParams, sizeof(gpHci_LeMetaDataLengthChange_t));

    gpBle_SendEvent(pEventBuf);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_DataLengthExchangeInit(void)
{
    Ble_LlcpRegisterDataLengthExchangeProcedure();
}

void gpBleLlcpProcedures_DataLengthExchangeReset(void)
{
    // Reset last data length storage buffer
    Ble_LlcpLastDataLengthChange.connectionHandle = GP_HCI_CONNECTION_HANDLE_INVALID;
    Ble_LlcpLastDataLengthChange.MaxRxOctets = 0;
    Ble_LlcpLastDataLengthChange.MaxRxTime = 0;
    Ble_LlcpLastDataLengthChange.MaxTxOctets = 0;
    Ble_LlcpLastDataLengthChange.MaxTxTime = 0;
}

