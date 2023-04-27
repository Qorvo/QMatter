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

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLELLCPPROCEDURES

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpHci_Includes.h"
#include "gpBle.h"
#include "gpBleDataChannelRxQueue.h"
#include "gpBleDataChannelTxQueue.h"
#include "gpBleDataRx.h"
#include "gpBleDataTx.h"
#include "gpBleLlcp.h"
#include "gpBleLlcpFramework.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcpProcedures_defs.h"
#include "gpBleSecurityCoprocessor.h"
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpRandom.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

// Expected encryption PDU payload lengths
#define BLE_LLCP_PDU_PAYLOAD_LENGTH_ENC_REQ         22
#define BLE_LLCP_PDU_PAYLOAD_LENGTH_ENC_RSP         12
#define BLE_LLCP_PDU_PAYLOAD_LENGTH_START_ENC_REQ    0
#define BLE_LLCP_PDU_PAYLOAD_LENGTH_START_ENC_RSP    0
#define BLE_LLCP_PDU_PAYLOAD_LENGTH_PAUSE_ENC_REQ    0
#define BLE_LLCP_PDU_PAYLOAD_LENGTH_PAUSE_ENC_RSP    0


/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/



typedef struct {
    gpBleLlcp_Opcode_t opcode;
    UInt8 length;
} Ble_LlcpEncPduLengthMapping_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

// Checker/Action functions
static gpHci_Result_t Ble_LongTermKeyRequestReplyAction(Ble_IntConnId_t connId, gpHci_LeLongTermKeyRequestReplyCommand_t * pLtkParams);

// Encryption start/pause
static Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpEncryptiontGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData);
static Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionUnexpectedPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh);
static Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionPduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static void Ble_LlcpEncryptionStartFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpEncryptionPauseFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);

static void Ble_LlcpRegisterEncryptionStartProcedure(void);
static void Ble_LlcpRegisterEncryptionPauseProcedure(void);

// Various
static Ble_LlcpProcedureContext_t* Ble_LlcpGetActiveEncryptionProcedure(Ble_LlcpLinkContext_t* pContext);
static void Ble_LlcpCalculateSecurityKey(Ble_LlcpLinkContext_t* pContext, UInt8* pLtk);
static void BleLlcpProcedures_cbUnexpectedPduReceived(Ble_IntConnId_t connId);

#ifdef GP_LOCAL_LOG
static void BleLlcpProcedures_DumpLtk(UInt8* pLtk);
#endif //GP_LOCAL_LOG

/*****************************************************************************
 *                    Procedure descriptors
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_EncryptionStartPduDescriptors[] =
{
    {gpBleLlcp_OpcodeEncReq, 22, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_CENTRAL},
    {gpBleLlcp_OpcodeEncRsp, 12, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_PERIPHERAL},
    {gpBleLlcp_OpcodeStartEncReq, 0, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_PERIPHERAL},
    {gpBleLlcp_OpcodeStartEncRsp, 0, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH}
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_EncryptionStartDescriptor =
{
    .procedureFlags = 0x00,
    .procedureDataLength = sizeof(Ble_EnableEncryptionProcedureData_t),
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_EncryptionStartPduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_EncryptionStartPduDescriptors,
    .featureMask = BM(gpBleConfig_FeatureIdLeEncryption),
    .cbQueueingNeeded = NULL,
    .cbProcedureStart = Ble_LlcpEncryptionStart,
    .cbGetCtrData = Ble_LlcpEncryptiontGetCtrData,
    .cbStoreCtrData = Ble_LlcpEncryptionStoreCtrData,
    .cbPduReceived = Ble_LlcpEncryptionPduReceived,
    .cbUnexpectedPduReceived = Ble_LlcpEncryptionUnexpectedPduReceived,
    .cbPduQueued = Ble_LlcpEncryptionPduQueued,
    .cbPduTransmitted = Ble_LlcpEncryptionPduTransmitted,
    .cbFinished = Ble_LlcpEncryptionStartFinished
};

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_EncryptionPausePduDescriptors[] =
{
    {gpBleLlcp_OpcodePauseEncReq, 0, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_CENTRAL},
    {gpBleLlcp_OpcodePauseEncRsp, 0, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH}
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_EncryptionPauseDescriptor =
{
    .procedureFlags = 0x00,
    .procedureDataLength = 0,
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_EncryptionPausePduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_EncryptionPausePduDescriptors,
    .featureMask = BM(gpBleConfig_FeatureIdLeEncryption),
    .cbQueueingNeeded = NULL,
    .cbProcedureStart = Ble_LlcpEncryptionStart,
    .cbGetCtrData = Ble_LlcpEncryptiontGetCtrData,
    .cbStoreCtrData = Ble_LlcpEncryptionStoreCtrData,
    .cbPduReceived = Ble_LlcpEncryptionPduReceived,
    .cbUnexpectedPduReceived = Ble_LlcpEncryptionUnexpectedPduReceived,
    .cbPduQueued = Ble_LlcpEncryptionPduQueued,
    .cbPduTransmitted = Ble_LlcpEncryptionPduTransmitted,
    .cbFinished = Ble_LlcpEncryptionPauseFinished
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

gpHci_Result_t Ble_LongTermKeyRequestReplyAction(Ble_IntConnId_t connId, gpHci_LeLongTermKeyRequestReplyCommand_t * pLtkParams)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;
    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext);

    pProcedure = gpBleLlcpFramework_GetProcedure(pContext, false);

    if( pProcedure == NULL || !gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_HOST_IDX) || pProcedure->procedureId != gpBleLlcp_ProcedureIdEncryptionStart)
    {
        GP_LOG_PRINTF("LTK reply for conn %x not allowed, no pending procedure",0,pContext->connId);
        // Not allowed to issue this command if the encryption start procedure is not active
        return gpHci_ResultCommandDisallowed;
    }

    gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_HOST_IDX);

    if(pLtkParams != NULL)
    {
        Ble_LlcpCalculateSecurityKey(pContext, pLtkParams->longTermKey);

#ifdef GP_LOCAL_LOG
    BleLlcpProcedures_DumpLtk(pLtkParams->longTermKey);
#endif //GP_LOCAL_LOG

        // From this point, we need to decrypt RX PDUs
        gpBle_DataRxQueueEnableDecryption(pContext->connId, true);

        gpBleLlcpFramework_ResumeProcedure(pContext, pProcedure, gpBleLlcp_OpcodeStartEncReq);
    }
    else
    {
        pProcedure->result =  gpHci_ResultPINorKeyMissing;
        gpBleLlcpFramework_ResumeProcedure(pContext, pProcedure, gpBleLlcp_OpcodeRejectExtInd);
    }

    return gpHci_ResultSuccess;
}

// Function is called for both control and data PDUs
void BleLlcpProcedures_cbUnexpectedPduReceived(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;
    GP_LOG_SYSTEM_PRINTF("cbUnexpectedPduReceived => stop connection",0);

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    pProcedure = Ble_LlcpGetActiveEncryptionProcedure(pContext);
    GP_ASSERT_DEV_INT(pProcedure != NULL);

    pProcedure->result = gpHci_ResultConnectionTerminatedduetoMICFailure;
    gpBle_StopConnection(pContext->hciHandle, pProcedure->result);
}

#ifdef GP_LOCAL_LOG
void BleLlcpProcedures_DumpLtk(UInt8* pLtk)
{
     GP_LOG_SYSTEM_PRINTF("LTK MSB first: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",0,
        pLtk[0], pLtk[1], pLtk[2], pLtk[3], pLtk[4], pLtk[5], pLtk[6], pLtk[7], pLtk[8], pLtk[9], pLtk[10], pLtk[11], pLtk[12], pLtk[13], pLtk[14], pLtk[15]);

    GP_LOG_SYSTEM_PRINTF("LTK LSB first: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",0,
        pLtk[15], pLtk[14], pLtk[13], pLtk[12], pLtk[11], pLtk[10], pLtk[9], pLtk[8], pLtk[7], pLtk[6], pLtk[5], pLtk[4], pLtk[3], pLtk[2], pLtk[1], pLtk[0]);
}
#endif //GP_LOCAL_LOG

//---------------------------
// Encryption start/pause procedure
//---------------------------

Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    Ble_LlcpFrameworkAction_t frameworkAction = Ble_LlcpFrameworkActionContinue;

    GP_ASSERT_DEV_INT(pContext != NULL);

    // Only when encryption is started for first time or when encryption is paused
    if(!pContext->encryptionEnabled || pProcedure->procedureId == gpBleLlcp_ProcedureIdEncryptionPause)
    {
        if(pContext->masterConnection)
        {
            gpBle_DataTxSetConnectionPause(pContext->connId, true);

            frameworkAction = Ble_LlcpFrameworkActionWaitForEmptyTxQueue;
        }
        else
        {
            // Slave needs to continue in order to store the RX data
            frameworkAction = Ble_LlcpFrameworkActionContinue;
        }
    }

    if(!pProcedure->localInit)
    {
        // Procedure init by remote, we can intercept unexpected pdu's now
        gpBle_DataRxInterceptUnexpectedPdus(pContext->connId, BleLlcpProcedures_cbUnexpectedPduReceived);
    }

    return frameworkAction;
}

void Ble_LlcpEncryptiontGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData)
{
    UInt8 offset = 0;

    GP_ASSERT_DEV_INT(pContext != NULL);

    if(*pOpcode == gpBleLlcp_OpcodeRejectExtInd && gpBleLlcp_IsFeatureSupported(pContext->hciHandle, gpBleConfig_FeatureIdExtendedRejectIndication) != gpBleLlcp_FeatureStatus_Supported)
    {
        // If remote does not support extended reject, we use reject
        *pOpcode = gpBleLlcp_OpcodeRejectInd;
    }

    switch(*pOpcode)
    {
        case gpBleLlcp_OpcodeEncReq:
        {
            gpHci_LeEnableEncryptionCommand_t startEncParams;
            Ble_EnableEncryptionProcedureData_t* pData;

            pData = (Ble_EnableEncryptionProcedureData_t*)pProcedure->pData;
            GP_ASSERT_DEV_INT(pData != NULL);

            // 8 byte rand
            MEMCPY(&pCtrData[offset], pData->randomNumber, GP_HCI_RANDOM_DATA_LENGTH);
            offset += GP_HCI_RANDOM_DATA_LENGTH;
            // 2 byte ediv
            MEMCPY(&pCtrData[offset], &pData->encryptedDiversifier, sizeof(pData->encryptedDiversifier));
            offset += sizeof(startEncParams.encryptedDiversifier);

            // 8 byte SKDm
            gpRandom_GetFromDRBG(BLE_SEC_KEY_DIV_PART_LENGTH, pData->sessionKeyDivMaster);
            MEMCPY(&pCtrData[offset], pData->sessionKeyDivMaster, BLE_SEC_KEY_DIV_PART_LENGTH);
            offset += BLE_SEC_KEY_DIV_PART_LENGTH;

            // 4 byte IVm
            gpRandom_GetFromDRBG(BLE_SEC_INIT_VECTOR_PART_LENGTH, pData->initVectorMaster);
            MEMCPY(&pCtrData[offset], pData->initVectorMaster, BLE_SEC_INIT_VECTOR_PART_LENGTH);
            offset += BLE_SEC_INIT_VECTOR_PART_LENGTH;

            break;
        }
        case gpBleLlcp_OpcodeEncRsp:
        {
            Ble_EnableEncryptionProcedureData_t* pData;

            pData = (Ble_EnableEncryptionProcedureData_t*)pProcedure->pData;
            GP_ASSERT_DEV_INT(pData != NULL);

            // 8 byte SKDs
            gpRandom_GetFromDRBG(BLE_SEC_KEY_DIV_PART_LENGTH, pData->sessionKeyDivSlave);
            MEMCPY(&pCtrData[offset], pData->sessionKeyDivSlave, BLE_SEC_KEY_DIV_PART_LENGTH);
            offset += BLE_SEC_KEY_DIV_PART_LENGTH;

            // 4 byte IVs
            gpRandom_GetFromDRBG(BLE_SEC_INIT_VECTOR_PART_LENGTH, pData->initVectorSlave);
            MEMCPY(&pCtrData[offset], pData->initVectorSlave, BLE_SEC_INIT_VECTOR_PART_LENGTH);
            offset += BLE_SEC_INIT_VECTOR_PART_LENGTH;

            break;
        }
        case gpBleLlcp_OpcodeRejectInd:
        {
            pCtrData[offset++] = gpHci_ResultPINorKeyMissing;
            break;
        }
        case gpBleLlcp_OpcodeRejectExtInd:
        {
            // || RejectOpcode (1) | Error code (1) ||
            pCtrData[offset++] = gpBleLlcp_OpcodeEncReq;
            pCtrData[offset++] = gpHci_ResultPINorKeyMissing;

            break;
        }
        // Opcodes without ctrData
        case gpBleLlcp_OpcodeStartEncReq:
        case gpBleLlcp_OpcodeStartEncRsp:
        case gpBleLlcp_OpcodePauseEncReq:
        case gpBleLlcp_OpcodePauseEncRsp:
        {
            break;
        }

        default:
        {
            // Should not happen
            GP_ASSERT_DEV_INT(false);
        }
    }
    *pCtrDataLength = offset;
}

Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;
    Ble_EnableEncryptionProcedureData_t* pData;

    pData = (Ble_EnableEncryptionProcedureData_t*)pProcedure->pData;

    switch(opcode)
    {
        case gpBleLlcp_OpcodeEncReq:
        {
            GP_ASSERT_DEV_INT(pData != NULL);

            gpPd_ReadWithUpdate(pPdLoh, GP_HCI_RANDOM_DATA_LENGTH, pData->randomNumber);
            gpPd_ReadWithUpdate(pPdLoh, sizeof(pData->encryptedDiversifier), (UInt8*)&pData->encryptedDiversifier);
            gpPd_ReadWithUpdate(pPdLoh, BLE_SEC_KEY_DIV_PART_LENGTH, pData->sessionKeyDivMaster);
            gpPd_ReadWithUpdate(pPdLoh, BLE_SEC_INIT_VECTOR_PART_LENGTH, pData->initVectorMaster);

            break;
        }
        case gpBleLlcp_OpcodeEncRsp:
        {
            GP_ASSERT_DEV_INT(pData != NULL);

            gpPd_ReadWithUpdate(pPdLoh, BLE_SEC_KEY_DIV_PART_LENGTH, pData->sessionKeyDivSlave);
            gpPd_ReadWithUpdate(pPdLoh, BLE_SEC_INIT_VECTOR_PART_LENGTH, pData->initVectorSlave);

            break;
        }
        case gpBleLlcp_OpcodeStartEncReq:
        {
            break;
        }
        case gpBleLlcp_OpcodeStartEncRsp:
        {
            break;
        }
        case gpBleLlcp_OpcodePauseEncReq:
        {
            break;
        }
        case gpBleLlcp_OpcodePauseEncRsp:
        {
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

Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    Ble_LlcpFrameworkAction_t frameworkAction = Ble_LlcpFrameworkActionContinue;

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodeEncReq:
        {
            if(!pContext->encryptionEnabled)
            {
                // Pause Tx data (do not allow new PDU's to be added)
                // If encryption was enabled before, pause tx data is done by encryption pause procedure
                gpBle_DataTxSetConnectionPause(pContext->connId, true);


                // We need to suspend local procedures during encryption procedure
                // If encryption was enabled before, suspend is done by encryption pause procedure
                gpBleLlcpFramework_EnableProcedureHandling(pContext, true, false);
            }

            frameworkAction = Ble_LlcpFrameworkActionWaitForEmptyTxQueue;

            pProcedure->currentTxPdu = gpBleLlcp_OpcodeEncRsp;

            break;
        }
        case gpBleLlcp_OpcodeEncRsp:
        {
            // Pause (wait for start enc req from slave)
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeStartEncReq;

            frameworkAction = Ble_LlcpFrameworkActionPause;

            if(pContext->masterConnection)
            {
                gpBle_DataRxInterceptUnexpectedPdus(pContext->connId, BleLlcpProcedures_cbUnexpectedPduReceived);
            }

            break;
        }
        case gpBleLlcp_OpcodeStartEncReq:
        {
            Ble_LlcpCalculateSecurityKey(pContext, NULL);

            // From now on, we encrypt all TX data on this link
            gpBle_DataTxQueueEnableEncryption(pContext->connId, true);

            // From now on, we decrypt all RX data on this link
            gpBle_DataRxQueueEnableDecryption(pContext->connId, true);

            pProcedure->currentTxPdu = gpBleLlcp_OpcodeStartEncRsp;

            break;
        }
        case gpBleLlcp_OpcodeStartEncRsp:
        {
            if(pContext->masterConnection)
            {
                frameworkAction = Ble_LlcpFrameworkActionStop;
            }
            else
            {
                gpBle_DataTxQueueEnableEncryption(pContext->connId, true);
                pProcedure->currentTxPdu = gpBleLlcp_OpcodeStartEncRsp;
            }
            break;
        }
        case gpBleLlcp_OpcodePauseEncReq:
        {
            // Pause Tx data (do not allow new PDU's to be added)
            gpBle_DataTxSetConnectionPause(pContext->connId, true);

            // As slave, we need to suspend local procedures during encryption pause procedure
            gpBleLlcpFramework_EnableProcedureHandling(pContext, true, false);

            frameworkAction = Ble_LlcpFrameworkActionWaitForEmptyTxQueue;

            pProcedure->currentTxPdu = gpBleLlcp_OpcodePauseEncRsp;
            break;
        }
        case gpBleLlcp_OpcodePauseEncRsp:
        {
            gpBle_DataTxQueueEnableEncryption(pContext->connId, false);
            gpBle_DataRxQueueEnableDecryption(pContext->connId, false);

            if(pContext->masterConnection)
            {
                gpBle_DataRxInterceptUnexpectedPdus(pContext->connId, BleLlcpProcedures_cbUnexpectedPduReceived);
                pProcedure->currentTxPdu = gpBleLlcp_OpcodePauseEncRsp;
            }
            else
            {
                frameworkAction = Ble_LlcpFrameworkActionStop;
            }

            break;
        }
        default:
        {
            // We should not end up here (handling done in Ble_LlcpEncryptionUnexpectedPduReceived)
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return frameworkAction;
}

Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionUnexpectedPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh)
{
    Ble_LlcpFrameworkAction_t frameworkAction = Ble_LlcpFrameworkActionContinue;

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodeRejectInd:
        case gpBleLlcp_OpcodeRejectExtInd:
        {
            gpBleLlcp_Opcode_t rejectOpcode = gpBleLlcp_OpcodeInvalid;
            gpHci_Result_t rejectErrorCode;

            if(rxOpcode == gpBleLlcp_OpcodeRejectInd)
            {
                // we don't know the rejectOpcode, assume gpBleLlcp_OpcodeEncReq
                rejectOpcode = gpBleLlcp_OpcodeEncReq;
                rejectErrorCode = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset);
            }
            else
            {
                rejectOpcode = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset);
                rejectErrorCode = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset+1);
            }

            if(rejectOpcode == gpBleLlcp_OpcodeEncReq)
            {
                if(rejectErrorCode == gpHci_ResultPINorKeyMissing || rejectErrorCode == gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature)
                {
                    // These are the only error codes that can be send in the context of the encryption procedure
                    pProcedure->result = rejectErrorCode;
                    frameworkAction = Ble_LlcpFrameworkActionStop;
                }
            }

            break;
        }
        case gpBleLlcp_OpcodeUnknownRsp:
        {
            if(pContext->masterConnection)
            {
                // Unknown response not allowed when local device is in master role ==> disconnect
                frameworkAction = Ble_LlcpFrameworkActionPause;
                BleLlcpProcedures_cbUnexpectedPduReceived(pContext->connId);
            }
            else
            {
                gpBleLlcp_Opcode_t unknownOpcode;

                unknownOpcode = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset);

                if(pContext->pProcedureLocal && pContext->pProcedureLocal->currentTxPdu == unknownOpcode)
                {
                    // We have a local procedure and the unknownOpcode matches the opcode we sent in the request, allow this PDU
                    frameworkAction = Ble_LlcpFrameworkActionContinue;
                }
                else
                {
                    // No local procedure, or wrong opcode, disconnect
                    frameworkAction = Ble_LlcpFrameworkActionPause;
                    BleLlcpProcedures_cbUnexpectedPduReceived(pContext->connId);
                }
            }

            break;
        }
        default:
        {
            if(pContext->masterConnection && (pProcedure->expectedRxPdu == gpBleLlcp_OpcodeEncRsp || pProcedure->expectedRxPdu == gpBleLlcp_OpcodePauseEncRsp))
            {
                frameworkAction = Ble_LlcpFrameworkActionContinue;
            }
            else
            {
                // Not allowed, disconnect
                frameworkAction = Ble_LlcpFrameworkActionPause;
                BleLlcpProcedures_cbUnexpectedPduReceived(pContext->connId);
            }
            break;
        }
    }

    return frameworkAction;
}

Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodeEncReq:
        {
            if(!pContext->encryptionEnabled)
            {
                // At this point, we should suspend the remote procedure, but only if encryption is performed the first time
                gpBleLlcpFramework_EnableProcedureHandling(pContext, false, false);
            }
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeEncRsp;
            break;
        }
        case gpBleLlcp_OpcodeEncRsp:
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeStartEncRsp;
            break;
        }
        case gpBleLlcp_OpcodeStartEncReq:
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeStartEncRsp;
            break;
        }
        case gpBleLlcp_OpcodeStartEncRsp:
        {
            if(pContext->masterConnection)
            {
                pProcedure->expectedRxPdu = gpBleLlcp_OpcodeStartEncRsp;
            }
            else
            {
                pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;

                action = Ble_LlcpFrameworkActionStop;
            }
            break;
        }
        case gpBleLlcp_OpcodePauseEncReq:
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodePauseEncRsp;

            // At this point, we should suspend the remote procedure
            gpBleLlcpFramework_EnableProcedureHandling(pContext, false, false);
            break;
        }
        case gpBleLlcp_OpcodePauseEncRsp:
        {
            gpBle_DataRxQueueEnableDecryption(pContext->connId, false);

            if(pContext->masterConnection)
            {
                pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;
                action = Ble_LlcpFrameworkActionStop;
            }
            else
            {
                pProcedure->expectedRxPdu = gpBleLlcp_OpcodePauseEncRsp;
            }
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

Ble_LlcpFrameworkAction_t Ble_LlcpEncryptionPduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodeEncReq:
        {
            break;
        }
        case gpBleLlcp_OpcodeEncRsp:
        {
            if(!pContext->masterConnection)
            {

                Ble_EnableEncryptionProcedureData_t* pStartEncData;
                gpHci_EventCbPayload_t params;

                action = Ble_LlcpFrameworkActionPause;

                pStartEncData = (Ble_EnableEncryptionProcedureData_t*)pProcedure->pData;

                GP_ASSERT_DEV_INT(pStartEncData != NULL);

                params.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeLongTermKeyRequest;

                params.metaEventParams.params.longTermKeyRequest.connectionHandle = pContext->hciHandle;
                MEMCPY(params.metaEventParams.params.longTermKeyRequest.randomNumber, pStartEncData->randomNumber, GP_HCI_RANDOM_DATA_LENGTH);
                MEMCPY(&params.metaEventParams.params.longTermKeyRequest.encryptedDiversifier, &pStartEncData->encryptedDiversifier, sizeof(pStartEncData->encryptedDiversifier));

                gpBle_ScheduleEvent(0, gpHci_EventCode_LEMeta, &params);

                gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_HOST_IDX);
            }
            break;
        }
        case gpBleLlcp_OpcodeStartEncReq:
        {
            break;
        }
        case gpBleLlcp_OpcodeStartEncRsp:
        {
            break;
        }
        case gpBleLlcp_OpcodePauseEncReq:
        {
            break;
        }
        case gpBleLlcp_OpcodePauseEncRsp:
        {
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

void Ble_LlcpEncryptionStartFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    gpHci_EventCode_t eventCode;
    gpHci_EventCbPayload_t params;

    gpBle_DataTxSetConnectionPause(pContext->connId, false);
    gpBle_DataRxInterceptUnexpectedPdus(pContext->connId, NULL);

    if(pContext->masterConnection)
    {
        // Re-enable remote procedure flow
        gpBleLlcpFramework_EnableProcedureHandling(pContext, false, true);
    }
    else
    {
        // Re-enable local procedure flow
        gpBleLlcpFramework_EnableProcedureHandling(pContext, true, true);
    }

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
    if(pProcedure->result == gpHci_ResultSuccess)
    {
        gpBle_DataRxQueueEnableAuthPayloadTo(pContext->connId, true);
    }
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED

    if(pContext->encryptionEnabled)
    {
        eventCode = gpHci_EventCode_EncryptionKeyRefreshComplete;
        params.keyRefreshComplete.connectionHandle = pContext->hciHandle;
        params.keyRefreshComplete.status = pProcedure->result;
    }
    else
    {
        eventCode = gpHci_EventCode_EncryptionChange;
        params.encryptionChangeParams.connectionHandle = pContext->hciHandle;
        params.encryptionChangeParams.status = pProcedure->result;

        if(pProcedure->result == gpHci_ResultSuccess)
        {
            // Mark this link as encrypted (needed for restarting encryption)
            pContext->encryptionEnabled = true;
        }

         params.encryptionChangeParams.encryptionEnabled = (pContext->encryptionEnabled ? gpHci_EncryptionLevelOn: gpHci_EncryptionLevelOff);
    }

    if(!notifyHost)
    {
        return;
    }

    if((pContext->masterConnection || pContext->encryptionEnabled) && pProcedure->result != gpHci_ResultConnectionTerminatedduetoMICFailure)
    {
        /* Only notify host in case:
         * - we are master or when encryption was success.
         * - the result is different from 0x3D (gpHci_ResultConnectionTerminatedduetoMICFailure).
             See LLTS 4.2.2 section 4.8.6.14
         */
        gpBle_ScheduleEvent(0, eventCode, &params);
    }
}

void Ble_LlcpEncryptionPauseFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
    if(pProcedure->result == gpHci_ResultSuccess)
    {
        gpBle_DataRxQueueEnableAuthPayloadTo(pContext->connId, false);
    }
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
}

void Ble_LlcpRegisterEncryptionStartProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdEncryptionStart, &BleLlcpProcedures_EncryptionStartDescriptor);

    gpBleLlcpFramework_RegisterInvalidProcedureAction(gpBleLlcp_ProcedureIdEncryptionStart, false);
}

void Ble_LlcpRegisterEncryptionPauseProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdEncryptionPause, &BleLlcpProcedures_EncryptionPauseDescriptor);
    gpBleLlcpFramework_RegisterInvalidProcedureAction(gpBleLlcp_ProcedureIdEncryptionPause, false);
}

Ble_LlcpProcedureContext_t* Ble_LlcpGetActiveEncryptionProcedure(Ble_LlcpLinkContext_t* pContext)
{
    Ble_LlcpProcedureContext_t* pProcedure;
    Bool local = pContext->masterConnection;

    pProcedure = gpBleLlcpFramework_GetProcedure(pContext, local);

    if(pProcedure == NULL)
    {
        return NULL;
    }

    if(pProcedure->procedureId == gpBleLlcp_ProcedureIdEncryptionStart || pProcedure->procedureId == gpBleLlcp_ProcedureIdEncryptionPause)
    {
        return pProcedure;
    }

    return NULL;
}

void Ble_LlcpCalculateSecurityKey(Ble_LlcpLinkContext_t* pContext, UInt8* pLtk)
{
    Ble_LlcpProcedureContext_t* pProcedure;
    Ble_EnableEncryptionProcedureData_t* pStartEncData;
    UInt8 sessionKeyDiv[GP_HCI_ENCRYPTION_KEY_LENGTH];
    UInt8 initVector[BLE_SEC_INIT_VECTOR_LENGTH];
    Bool local = pContext->masterConnection;

    pProcedure = gpBleLlcpFramework_GetProcedure(pContext, local);

    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pProcedure->procedureId == gpBleLlcp_ProcedureIdEncryptionStart);

    pStartEncData = (Ble_EnableEncryptionProcedureData_t*)pProcedure->pData;

    if(!pContext->masterConnection)
    {
        if(pLtk == NULL)
        {
            return;
        }

        // Slave does not have the LTK yet, store it from parameter
        MEMCPY(pStartEncData->longTermKey, pLtk, GP_HCI_ENCRYPTION_KEY_LENGTH);
    }

    // Calculate SKD (Session Key Diversifier)
    MEMCPY(&sessionKeyDiv[0], pStartEncData->sessionKeyDivMaster, BLE_SEC_KEY_DIV_PART_LENGTH);
    MEMCPY(&sessionKeyDiv[BLE_SEC_KEY_DIV_PART_LENGTH], pStartEncData->sessionKeyDivSlave, BLE_SEC_KEY_DIV_PART_LENGTH);

    // Calculate IV (Initialisation Vector)
    MEMCPY(&initVector[0], pStartEncData->initVectorMaster, BLE_SEC_INIT_VECTOR_PART_LENGTH);
    MEMCPY(&initVector[BLE_SEC_INIT_VECTOR_PART_LENGTH], pStartEncData->initVectorSlave, BLE_SEC_INIT_VECTOR_PART_LENGTH);

    gpBle_SecurityCoprocessorCalculateSessionKey(pContext->connId, pStartEncData->longTermKey, sessionKeyDiv, initVector);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_EncryptionInit(void)
{
    Ble_LlcpRegisterEncryptionStartProcedure();
    Ble_LlcpRegisterEncryptionPauseProcedure();
}

/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_LeLongTermKeyRequestReply(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    gpHci_LeLongTermKeyRequestReplyCommand_t * pLtkParams = &pParams->LeLongTermKeyRequestReply;

    GP_LOG_PRINTF("Long Term Key Request Reply for conn: %x",0,pLtkParams->connectionHandle);

    // Use Command complete event to reply to a long term key request reply command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    pEventBuf->payload.commandCompleteParams.returnParams.connectionHandle = pLtkParams->connectionHandle;

    result = gpBleLlcp_IsHostConnectionHandleValid(pLtkParams->connectionHandle);

    if(result == gpHci_ResultSuccess)
    {
        result = Ble_LongTermKeyRequestReplyAction(gpBleLlcp_HciHandleToIntHandle(pLtkParams->connectionHandle), pLtkParams);
    }

    return result;
}

gpHci_Result_t gpBle_LeLongTermKeyRequestNegativeReply(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    gpHci_LeLongTermKeyRequestNegativeReplyCommand_t * pLtkParams = &pParams->LeLongTermKeyRequestNegativeReply;

    GP_LOG_PRINTF("Long Term Key Request Negative Reply for conn: %x",0,pLtkParams->connectionHandle);

    if(pEventBuf != NULL)
    {
        // This check is needed, because this function can be called directly when the event is masked
        BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
        pEventBuf->payload.commandCompleteParams.returnParams.connectionHandle = pLtkParams->connectionHandle;
    }

    result = gpBleLlcp_IsHostConnectionHandleValid(pLtkParams->connectionHandle);

    if(result == gpHci_ResultSuccess)
    {
        result = Ble_LongTermKeyRequestReplyAction(gpBleLlcp_HciHandleToIntHandle(pLtkParams->connectionHandle), NULL);
    }

    return result;
}

