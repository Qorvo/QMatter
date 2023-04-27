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
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpSched.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

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

static Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesQueueingNeeded(Ble_LlcpLinkContext_t* pContext);
static Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpFeaturesGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* ctrDataLength, UInt8* pCtrData);
static Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static void Ble_LlcpFeaturesFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpFeaturesNotifyHost(Ble_LlcpLinkContext_t* pContext, gpHci_Result_t result);
static void Ble_LlcpRegisterFeatureExchangeProcedure(void);

/*****************************************************************************
 *                    Procedure descriptor
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_FeatureExchangePduDescriptors[] =
{
    {gpBleLlcp_OpcodeFeatureReq, 8, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_CENTRAL},
    {gpBleLlcp_OpcodeSlaveFeatureReq, 8, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_PERIPHERAL},
    {gpBleLlcp_OpcodeFeatureRsp, 8, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH}
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_FeatureExchangeDescriptor =
{
    .procedureFlags = 0x00,
    .procedureDataLength = 0,
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_FeatureExchangePduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_FeatureExchangePduDescriptors,
    .featureMask = GPBLELLCP_FEATUREMASK_NONE,
    .cbQueueingNeeded = Ble_LlcpFeaturesQueueingNeeded,
    .cbProcedureStart = Ble_LlcpFeaturesStart,
    .cbGetCtrData = Ble_LlcpFeaturesGetCtrData,
    .cbStoreCtrData = Ble_LlcpFeaturesStoreCtrData,
    .cbPduReceived = Ble_LlcpFeaturesPduReceived,
    .cbUnexpectedPduReceived = Ble_LlcpCommonUnexpectedPduReceived,
    .cbPduQueued = Ble_LlcpFeaturesPduQueued,
    .cbPduTransmitted = NULL,
    .cbFinished = Ble_LlcpFeaturesFinished
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesQueueingNeeded(Ble_LlcpLinkContext_t* pContext)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    if(pContext->featuresExchangedStatus)
    {
        GP_LOG_PRINTF("No need to queue features, info cached",0);

        // Notify host
        Ble_LlcpFeaturesNotifyHost(pContext, pContext->featuresExchangedStatus == gpBleLlcp_FeatureStatus_ExchangedSuccess ? gpHci_ResultSuccess : gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature);
        action = Ble_LlcpFrameworkActionStop;
    }

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

#ifndef GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED
    if(pProcedure->localInit && !pContext->masterConnection)
    {
        GP_LOG_PRINTF("Slave feature exchange not supported",0);
        pProcedure->result = gpHci_ResultUnsupportedFeatureOrParameterValue;
        action = Ble_LlcpFrameworkActionStop;
    }
#endif //GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED

    if(pContext->featuresExchangedStatus && pProcedure->localInit)
    {
        GP_LOG_PRINTF("Features already exchanged. Return cached",0);
        pProcedure->result = pContext->featuresExchangedStatus == gpBleLlcp_FeatureStatus_ExchangedSuccess ? gpHci_ResultSuccess : gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature;
        action = Ble_LlcpFrameworkActionStop;
    }

    return action;
}

void Ble_LlcpFeaturesGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* ctrDataLength, UInt8* pCtrData)
{
    UInt8 offset = 0;
    UInt8* pFeaturesLocal;
    UInt64 tempFeaturesLocal;

    gpBleConfig_GetLocalSupportedFeatures(&tempFeaturesLocal);

    // Use UInt8 pointers for easier calculations
    pFeaturesLocal = (UInt8*)&tempFeaturesLocal;

    switch(*pOpcode)
    {
        case gpBleLlcp_OpcodeFeatureReq:
        {
            if(!pContext->masterConnection)
            {
#ifdef GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED
                // Slaves shall use gpBleLlcp_OpcodeSlaveFeatureReq
                *pOpcode = gpBleLlcp_OpcodeSlaveFeatureReq;
#else
                // Should not happen
                GP_ASSERT_DEV_INT(false);
#endif
            }

            MEMCPY(pCtrData, pFeaturesLocal, GP_HCI_FEATURE_SET_SIZE);
            offset += GP_HCI_FEATURE_SET_SIZE;

            break;
        }
        case gpBleLlcp_OpcodeFeatureRsp:
        {
            // First byte should always be featureSetUsed
            pCtrData[0] =  (pContext->featureSetLink & 0xFF);

#if GP_DIVERSITY_BLECONFIG_VERSION_ID <= gpBleConfig_BleVersionId_4_2
            // Before 5.0, byte 1-7 shall contain the featureSetAND (can be zero, as only first byte contains 4.2 features)
            MEMSET(&pCtrData[1], 0, GP_HCI_FEATURE_SET_SIZE-1);
#else
            // Starting from 5.0, byte 1-7 shall contain the local supported features
            MEMCPY(&pCtrData[1], &pFeaturesLocal[1], GP_HCI_FEATURE_SET_SIZE-1);
#endif
            offset += GP_HCI_FEATURE_SET_SIZE;

            break;
        }
        default:
        {
            // Should not happen
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    *ctrDataLength = offset;
}

Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    UInt8* pFeaturesLocal;
    UInt8* pFeatureSetLink;
    UInt64 tempFeaturesLocal;

    gpBleConfig_GetLocalSupportedFeatures(&tempFeaturesLocal);

    // Use UInt8 pointers for easier calculations
    pFeaturesLocal = (UInt8*)&tempFeaturesLocal;
    pFeatureSetLink = (UInt8*)&pContext->featureSetLink;

    switch(opcode)
    {
        case gpBleLlcp_OpcodeFeatureReq:
#ifdef GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED
        case gpBleLlcp_OpcodeSlaveFeatureReq:
#endif //GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED
        {
            UInt8 featureSetRemote[GP_HCI_FEATURE_SET_SIZE];

            gpPd_ReadWithUpdate(pPdLoh, GP_HCI_FEATURE_SET_SIZE, featureSetRemote);

            // First byte should always be featureSetUsed
            pFeatureSetLink[0] = (pFeaturesLocal[0] & featureSetRemote[0]);

#if GP_DIVERSITY_BLECONFIG_VERSION_ID >= gpBleConfig_BleVersionId_5_0
            // Byte 1-7 contain the remote features, can be left zero for < 5.0 devices
            MEMCPY(&pFeatureSetLink[1], &featureSetRemote[1], GP_HCI_FEATURE_SET_SIZE - 1);
#endif

            // Make sure we do not just MEMCPY pContext->featureSetLink over pContext->allowedProcedures
            // The framework wil clear a bit in allowedProcedures when it is not supported.
            pContext->allowedProcedures &= pContext->featureSetLink;

            break;
        }
        case gpBleLlcp_OpcodeFeatureRsp:
        {
            gpPd_ReadWithUpdate(pPdLoh, GP_HCI_FEATURE_SET_SIZE, pFeatureSetLink);

            // First byte should be featureSetUsed, rest should remain remote supported features
            // Make sure to AND ourselves in case the remote has not AND-ed the first byte
            pFeatureSetLink[0] = (pFeatureSetLink[0] & pFeaturesLocal[0]);

#if GP_DIVERSITY_BLECONFIG_VERSION_ID < gpBleConfig_BleVersionId_5_0
            // In case we are a pre 5.0 implementation, byte 1-7 should be 0 (as we don't have features in these bytes)
            MEMSET(&pFeatureSetLink[1], 0, GP_HCI_FEATURE_SET_SIZE - 1);
#endif

            // Make sure we do not just MEMCPY pContext->featureSetLink over pContext->allowedProcedures
            // The framework wil clear a bit in allowedProcedures when it is not supported.
            pContext->allowedProcedures &= pContext->featureSetLink;

            break;
        }
        default:
        {
            // Should not happen
            GP_LOG_PRINTF("Unwanted opCode:%x",0,opcode);
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return Ble_LlcpFrameworkActionContinue;
}

Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodeFeatureReq:
#ifdef GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED
        case gpBleLlcp_OpcodeSlaveFeatureReq:
#endif //GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeFeatureRsp;
            break;
        }
        case gpBleLlcp_OpcodeFeatureRsp:
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

Ble_LlcpFrameworkAction_t Ble_LlcpFeaturesPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    GP_ASSERT_DEV_INT(pProcedure != NULL);

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodeFeatureReq:
#ifdef GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED
        case gpBleLlcp_OpcodeSlaveFeatureReq:
#endif
        {
            pProcedure->currentTxPdu = gpBleLlcp_OpcodeFeatureRsp;
            break;
        }
        case gpBleLlcp_OpcodeFeatureRsp:
        {
            // We can stop when we have received the feature response
            action = Ble_LlcpFrameworkActionStop;
            break;
        }
        default:
        {
            // Should not be reached
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }
    return action;
}

void Ble_LlcpFeaturesFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    GP_LOG_PRINTF(" LlcpFeaturesFinished %d notifyHost %d  notify2 %d",2, pProcedure->result, notifyHost, (pProcedure->localInit && !pProcedure->controllerInit) );
    if(pProcedure->result == gpHci_ResultSuccess)
    {
        pContext->featuresExchangedStatus = gpBleLlcp_FeatureStatus_ExchangedSuccess;
    }
    /* if atleast one previous feature exchange was successful then mark the exchanged status as success (EBQ test) */
    else if(pContext->featuresExchangedStatus != gpBleLlcp_FeatureStatus_ExchangedSuccess)
    {
        pContext->featuresExchangedStatus = gpBleLlcp_FeatureStatus_ExchangedNoSuccess;
    }

    if(!notifyHost)
    {
        return;
    }

    if(pProcedure->localInit && !pProcedure->controllerInit)
    {
        // Notify host only in case it was the host on this device that triggered the command.
        Ble_LlcpFeaturesNotifyHost(pContext, pProcedure->result);
    }
}

void Ble_LlcpFeaturesNotifyHost(Ble_LlcpLinkContext_t* pContext, gpHci_Result_t result)
{
    gpHci_EventCbPayload_t params;

    MEMSET(&params.metaEventParams.params.readRemoteFeaturesComplete, 0, sizeof(gpHci_LEReadRemoteFeaturesCompleteParams_t));

    params.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeReadFeaturesComplete;
    params.metaEventParams.params.readRemoteFeaturesComplete.status = result;
    params.metaEventParams.params.readRemoteFeaturesComplete.connectionHandle = pContext->hciHandle;
    GP_LOG_PRINTF(" LlcpFeaturesNotifyHost result 0x%x " ,2, result);
    if(result == gpHci_ResultSuccess)
    {
        MEMCPY(params.metaEventParams.params.readRemoteFeaturesComplete.features, (UInt8*)&pContext->featureSetLink, GP_HCI_FEATURE_SET_SIZE);
    }

    // Needs to be scheduled, because it can be called directly from request function.
    // If not scheduled, the event will be send before the associated command status for the command
    gpBle_ScheduleEvent(0, gpHci_EventCode_LEMeta, &params);
}

void Ble_LlcpRegisterFeatureExchangeProcedure(void)
{
    // Register procedure and invalid pdu/role combos
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdFeatureExchange, &BleLlcpProcedures_FeatureExchangeDescriptor);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_FeatureExchangeInit(void)
{
    Ble_LlcpRegisterFeatureExchangeProcedure();
}

