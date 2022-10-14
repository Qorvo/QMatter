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
#include "gpBleInitiator.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcpProcedures_defs.h"
#include "gpBleLlcpProcedures_Update_defs.h"
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpHci_types_manual.h"
#include "gpSched.h"
#include "gpBle_PhyMask.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

// PDU payload lengths
#define BLE_CTR_DATA_LENGTH_PHY_REQ_RSP 2
#define BLE_CTR_DATA_LENGTH_PHY_UPDATE_IND 4

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_ALL_PHYS_USE_HOST_TX_PHYS(allPhys)  (!BIT_TST(allPhys, gpHci_PhyDirectionMask_Tx))
#define BLE_ALL_PHYS_USE_HOST_RX_PHYS(allPhys)  (!BIT_TST(allPhys, gpHci_PhyDirectionMask_Rx))

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
*****************************************************************************/

// Checker and action functions
static gpHci_Result_t Ble_LeSetDefaultPhyParamsChecker(gpHci_PhyDirectionMask_t allPhys, gpHci_PhyMask_t txPhys, gpHci_PhyMask_t rxPhys);
static void Ble_LeSetDefaultPhyAction(gpHci_LeSetDefaultPhyCommand_t* pParams);
static gpHci_Result_t Ble_LeSetPhyParamsChecker(gpHci_LeSetPhyCommand_t* pParams);
static gpHci_Result_t Ble_LeSetPhyAction(gpHci_LeSetPhyCommand_t* pParams);

static Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdateStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpPhyUpdateGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData);
static Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdateStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdatePduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdatePduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdatePduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static void Ble_LlcpPhyUpdateFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);

static void Ble_LlcpFilterMasterUpdateReqPhys(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpParamInvalidFollowUp(void* pArg);
static gpHci_Phy_t Ble_LlcpPhyUpdateMasterChoosePhy(gpHci_PhyMask_t phys);
static void Ble_LlcpRegisterPhyUpdateProcedure(void);
static gpHci_Phy_t Ble_LlcpPhyMaskToPhy(gpHci_PhyMask_t phyMask);
static Bool Ble_LlcpIsSymmetricConnectionRequested(gpHci_PhyMask_t txPhys, gpHci_PhyMask_t rxPhys);
static void Ble_LlcpPhyUpdateScheduleCompleteEvent(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t result, gpHci_Phy_t txPhy, gpHci_Phy_t rxPhy);
static void Ble_LlcpPhyUpdateCheckAndScheduleDataLengthChangeEvent(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
/*****************************************************************************
 *                    Procedure descriptor
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_PhyUpdatePduDescriptors[] =
{
    {gpBleLlcp_OpcodePhyReq, 2, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH},
    {gpBleLlcp_OpcodePhyRsp, 2, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_PERIPHERAL},
    {gpBleLlcp_OpcodePhyUpdateInd, 4, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_CENTRAL}
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_PhyUpdateDescriptor =
{
    .procedureFlags = GPBLELLCPFRAMEWORK_PROCEDURE_FLAGS_INSTANT_BM,
    .procedureDataLength = sizeof(Ble_LlcpPhyUpdateData_t),
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_PhyUpdatePduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_PhyUpdatePduDescriptors,
    .featureMask = BM(gpBleConfig_FeatureIdLe2MbitPhy) | BM(gpBleConfig_FeatureIdLeCodedPhy),
    .cbQueueingNeeded = NULL,
    .cbProcedureStart = Ble_LlcpPhyUpdateStart,
    .cbGetCtrData = Ble_LlcpPhyUpdateGetCtrData,
    .cbStoreCtrData = Ble_LlcpPhyUpdateStoreCtrData,
    .cbPduReceived = Ble_LlcpPhyUpdatePduReceived,
    .cbUnexpectedPduReceived = Ble_LlcpCommonUnexpectedPduReceived,
    .cbPduQueued = Ble_LlcpPhyUpdatePduQueued,
    .cbPduTransmitted = Ble_LlcpPhyUpdatePduTransmitted,
    .cbFinished = Ble_LlcpPhyUpdateFinished
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

gpHci_Result_t Ble_LeSetDefaultPhyParamsChecker(gpHci_PhyDirectionMask_t allPhys, gpHci_PhyMask_t txPhys, gpHci_PhyMask_t rxPhys)
{
    // If a phy is unsupported, an error should be returned
    if(BlePhy_IsUnsupportedPhyPresent(txPhys))
    {
        GP_LOG_PRINTF("Unsupported PHY in TX mask %x (sup: %x)",0, txPhys.mask, GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK().mask);
        return gpHci_ResultUnsupportedFeatureOrParameterValue;
    }

    if(BlePhy_IsUnsupportedPhyPresent(rxPhys))
    {
        GP_LOG_PRINTF("Unsupported PHY in RX mask %x (sup: %x)",0, rxPhys.mask, GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK().mask);
        return gpHci_ResultUnsupportedFeatureOrParameterValue;
    }

    if(BLE_ALL_PHYS_USE_HOST_TX_PHYS(allPhys))
    {
        if(HCI_PHYMASK_IS_ZERO(txPhys))
        {
            GP_LOG_PRINTF("Host did not specify valid TX PHYs",0);
            return gpHci_ResultInvalidHCICommandParameters;
        }
    }

    if(BLE_ALL_PHYS_USE_HOST_RX_PHYS(allPhys))
    {
        if(HCI_PHYMASK_IS_ZERO(rxPhys))
        {
            GP_LOG_PRINTF("Host did not specify valid RX PHYs",0);
            return gpHci_ResultInvalidHCICommandParameters;
        }
    }

    return gpHci_ResultSuccess;
}

void Ble_LeSetDefaultPhyAction(gpHci_LeSetDefaultPhyCommand_t* pParams)
{
    if(BLE_ALL_PHYS_USE_HOST_TX_PHYS(pParams->allPhys))
    {
        gpBle_SetDefaultPhyModesTx(pParams->txPhys);
    }
    else
    {
        gpBle_SetDefaultPhyModesTx(GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK());
    }

    if(BLE_ALL_PHYS_USE_HOST_RX_PHYS(pParams->allPhys))
    {
        gpBle_SetDefaultPhyModesRx(pParams->rxPhys);
    }
    else
    {
        gpBle_SetDefaultPhyModesRx(GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK());
    }
}

gpHci_Result_t Ble_LeSetPhyParamsChecker(gpHci_LeSetPhyCommand_t* pParams)
{
    if(gpBleLlcp_IsHostConnectionHandleValid(pParams->connectionHandle) != gpHci_ResultSuccess)
    {
        GP_LOG_PRINTF("Set phy unknown connhandle: %x",0, pParams->connectionHandle);
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    // Apart from connection handle and PHY options, the parameters are the same as the set default PHY command
    return Ble_LeSetDefaultPhyParamsChecker(pParams->allPhys, pParams->txPhys, pParams->rxPhys);
}

gpHci_Result_t Ble_LeSetPhyAction(gpHci_LeSetPhyCommand_t* pParams)
{
    gpHci_PhyMask_t txPhys = HCI_PHY_TO_PHYMASK(gpHci_Phy_None);
    gpHci_PhyMask_t rxPhys = HCI_PHY_TO_PHYMASK(gpHci_Phy_None);
    gpHci_Result_t result;
    Ble_IntConnId_t connId;

    connId = gpBleLlcp_HciHandleToIntHandle(pParams->connectionHandle);

    if(BLE_ALL_PHYS_USE_HOST_TX_PHYS(pParams->allPhys))
    {
        txPhys = pParams->txPhys;
        gpBle_SetPreferredPhyModesTx(connId, txPhys);
    }
    else
    {
        txPhys = gpBle_GetPreferredPhyModesTx(connId);
        gpBle_SetPreferredPhyModesTx(connId, GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK());
    }

    if(BLE_ALL_PHYS_USE_HOST_RX_PHYS(pParams->allPhys))
    {
        rxPhys = pParams->rxPhys;
        gpBle_SetPreferredPhyModesRx(connId, rxPhys);
    }
    else
    {
        rxPhys = gpBle_GetPreferredPhyModesRx(connId);
        gpBle_SetPreferredPhyModesRx(connId, GPBLEDATACOMMON_GET_SUPPORTED_PHYS_MASK());
    }

    if(!BLE_ALL_PHYS_USE_HOST_TX_PHYS(pParams->allPhys) && !BLE_ALL_PHYS_USE_HOST_RX_PHYS(pParams->allPhys))
    {
        // Host has no preference at all, don't start PHY update procedure (but we need to send a PHY update complete event in this case)
        result = gpHci_ResultSuccess;
        Ble_LlcpPhyUpdateScheduleCompleteEvent(pParams->connectionHandle, gpHci_ResultSuccess, gpBle_GetEffectivePhyTxType(connId), gpBle_GetEffectivePhyRxType(connId));
    }
    else
    {
        gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

        MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
        startDescriptor.procedureId = gpBleLlcp_ProcedureIdPhyUpdate;
        startDescriptor.procedureData.phyUpdate.txPhys = txPhys;
        startDescriptor.procedureData.phyUpdate.rxPhys = rxPhys;
        startDescriptor.procedureData.phyUpdate.selectedTxPhy = gpHci_Phy_Invalid;
        startDescriptor.procedureData.phyUpdate.selectedRxPhy = gpHci_Phy_Invalid;
        startDescriptor.procedureData.phyUpdate.phyOptions = pParams->phyOptions;

        result = gpBleLlcpFramework_StartProcedure(connId, &startDescriptor);
    }

    return result;
}

Bool Ble_PhyChangeRequested(gpHci_Phy_t currentPhy, gpHci_PhyMask_t possiblePhys)
{
    GP_ASSERT_DEV_INT(GP_HCI_PHY_TYPE_VALID(currentPhy));

    if(HCI_PHYMASK_IS_EQUAL(HCI_PHY_TO_PHYMASK(currentPhy), possiblePhys))
    {
        return false;
    }

    return true;
}

Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdateStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    Ble_LlcpPhyUpdateData_t *pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;

    GP_ASSERT_DEV_INT(pContext != NULL);

    GP_LOG_PRINTF("phy update start",0);

    if(!pProcedure->localInit)
    {
        // Contains intermediary result of PHY procedure.
        pData->txPhys = gpBle_GetPreferredPhyModesTx(pContext->connId);
        pData->rxPhys = gpBle_GetPreferredPhyModesRx(pContext->connId);
    }
    else
    {
        gpHci_Phy_t currentPhyTx = gpBle_GetEffectivePhyTxType(pContext->connId);
        gpHci_Phy_t currentPhyRx = gpBle_GetEffectivePhyRxType(pContext->connId);

        gpBle_DataTxSetConnectionPause(pContext->connId, true);

        /* Don't start the procedure when we know the Phy won't change.
         * We can only be certain the phy will not change if the current phy
         * matches the one in the PhyReq parameters. */
        if (!Ble_PhyChangeRequested(currentPhyTx, pData->txPhys) && !Ble_PhyChangeRequested(currentPhyRx, pData->rxPhys))
        {
            GP_LOG_PRINTF("Ble_LlcpPhyUpdateStart stop",0);
            pData->selectedTxPhy = gpHci_Phy_None;
            pData->selectedRxPhy = gpHci_Phy_None;
            return Ble_LlcpFrameworkActionStop;
        }

        return Ble_LlcpFrameworkActionWaitForEmptyTxQueue;
    }

    return Ble_LlcpFrameworkActionContinue;
}

void Ble_LlcpPhyUpdateGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData)
{
    Ble_LlcpPhyUpdateData_t * pData;

    pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;

    switch(*pOpcode)
    {
        case gpBleLlcp_OpcodePhyReq:
        case gpBleLlcp_OpcodePhyRsp:
        {
            // Same code for request and response
            gpBle_AppendWithUpdate(&pCtrData[*pCtrDataLength], &pData->txPhys.mask, pCtrDataLength, sizeof(pData->txPhys.mask));
            gpBle_AppendWithUpdate(&pCtrData[*pCtrDataLength], &pData->rxPhys.mask, pCtrDataLength, sizeof(pData->rxPhys.mask));
            break;
        }
        case gpBleLlcp_OpcodePhyUpdateInd:
        {
            UInt16 currentEventcount;
            gpHci_Phy_t m_to_s_phy;
            gpHci_Phy_t s_to_m_phy;
            gpHci_PhyMask_t m_to_s_mask = HCI_PHYMASK_INIT(0);
            gpHci_PhyMask_t s_to_m_mask = HCI_PHYMASK_INIT(0);

            m_to_s_phy = pData->selectedTxPhy;
            s_to_m_phy = pData->selectedRxPhy;

            GP_LOG_PRINTF("populate phys %x %x",0, m_to_s_phy, s_to_m_phy);

            pData->instant = 0;
            currentEventcount = gpHal_BleGetCurrentConnEventCount(pContext->connId);

            if ( gpHci_Phy_None != m_to_s_phy ||
                 gpHci_Phy_None != s_to_m_phy )
            {
                pData->instant = Ble_LlcpCalculateProcedureInstant(pContext, currentEventcount);
            }

            if(m_to_s_phy != gpHci_Phy_None)
            {
                m_to_s_mask = HCI_PHY_TO_PHYMASK(m_to_s_phy);
            }

            if(s_to_m_phy != gpHci_Phy_None)
            {
                s_to_m_mask = HCI_PHY_TO_PHYMASK(s_to_m_phy);
            }

            gpBle_AppendWithUpdate(&pCtrData[*pCtrDataLength], &m_to_s_mask.mask, pCtrDataLength, sizeof(m_to_s_mask.mask));
            gpBle_AppendWithUpdate(&pCtrData[*pCtrDataLength], &s_to_m_mask.mask, pCtrDataLength, sizeof(s_to_m_mask.mask));
            gpBle_AppendWithUpdate(&pCtrData[*pCtrDataLength], (UInt8*)&pData->instant, pCtrDataLength, sizeof(pData->instant));

            GP_LOG_PRINTF("PhyUpdateReq: 0x%02x 0x%02x 0x%04x",0,m_to_s_mask.mask, s_to_m_mask.mask, pData->instant);
            if (gpHci_Phy_None != m_to_s_phy ||
                gpHci_Phy_None != s_to_m_phy)
            {
                Ble_LlcpConfigureLastScheduledConnEventAfterCurrent(pProcedure, currentEventcount, pData->instant);
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
}

Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdateStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    Ble_LlcpPhyUpdateData_t* pData;
    UInt16 pduConnEventCount;

    pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;

    switch(opcode)
    {
        case gpBleLlcp_OpcodePhyReq:
        case gpBleLlcp_OpcodePhyRsp:
        {
            // Same handling for storing PHY REQ/PHY RSP PDUs
            gpPd_ReadWithUpdate(pPdLoh, 1, (UInt8*)&pData->txPhysRemote);
            gpPd_ReadWithUpdate(pPdLoh, 1, (UInt8*)&pData->rxPhysRemote);

            break;
        }
        case gpBleLlcp_OpcodePhyUpdateInd:
        {
            gpHci_PhyMask_t m_to_s_phy;
            gpHci_PhyMask_t s_to_m_phy;
            UInt16 instant = 0;

            gpPd_ReadWithUpdate(pPdLoh, 1, (UInt8*)&m_to_s_phy.mask);
            gpPd_ReadWithUpdate(pPdLoh, 1, (UInt8*)&s_to_m_phy.mask);
            gpPd_ReadWithUpdate(pPdLoh, 2, (UInt8*)&instant);

            pduConnEventCount = Ble_LlcpGetPduConnEventCount(pContext, pPdLoh);

            GP_LOG_PRINTF("Phy update IND RX: stom mtos %u %u",0,s_to_m_phy.mask ,m_to_s_phy.mask);

            pData->selectedTxPhy = Ble_LlcpPhyMaskToPhy(s_to_m_phy);
            pData->selectedRxPhy = Ble_LlcpPhyMaskToPhy(m_to_s_phy);

            pData->instant = instant;
            pData->pduConnEventCount = pduConnEventCount;

            break;
        }
        default:
        {
            // Should not happen
            GP_ASSERT_DEV_INT(false);
            return Ble_LlcpFrameworkActionStop;
            break;
        }
    }
    return Ble_LlcpFrameworkActionContinue;
}

Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdatePduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;
    Ble_LlcpPhyUpdateData_t * pData;

    pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodePhyReq:
        {
            GP_ASSERT_DEV_INT(!pProcedure->localInit);

            if(pContext->masterConnection)
            {
                Ble_LlcpFilterMasterUpdateReqPhys (pContext, pProcedure);
                pProcedure->currentTxPdu = gpBleLlcp_OpcodePhyUpdateInd;
            }
            else
            {
                pProcedure->currentTxPdu = gpBleLlcp_OpcodePhyRsp;
            }

            // Pause Tx data (do not allow new PDU's to be added)
            gpBle_DataTxSetConnectionPause(pContext->connId, true);

            action = Ble_LlcpFrameworkActionWaitForEmptyTxQueue;

            break;
        }
        case gpBleLlcp_OpcodePhyRsp:
        {
            Ble_LlcpFilterMasterUpdateReqPhys(pContext, pProcedure);

            pProcedure->currentTxPdu = gpBleLlcp_OpcodePhyUpdateInd;

            break;
        }
        case gpBleLlcp_OpcodePhyUpdateInd:
        {
            pProcedure->currentTxPdu = gpBleLlcp_OpcodeInvalid;
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;

            if (  gpHci_Phy_None == pData->selectedRxPhy &&
                  gpHci_Phy_None == pData->selectedTxPhy)
            {
                GP_LOG_PRINTF("PhyUpdateInd no change!",0);
                action = Ble_LlcpFrameworkActionStop;
            }
            else if (!Ble_LlcpInstantValid(pData->instant, pData->pduConnEventCount))
            {
                GP_LOG_PRINTF("Instant %u in past!",0, pData->instant);
                pProcedure->result = gpHci_ResultInstantPassed;
                action = Ble_LlcpFrameworkActionStop;
            }
            else if ( !Ble_IsPhyUpdateFieldValid(pData->selectedTxPhy) ||
                      !Ble_IsPhyUpdateFieldValid(pData->selectedRxPhy) )
            {
                GP_LOG_PRINTF("PhyUpdateReq invalid rxPhys|txPhys!",0);
                pProcedure->result = gpHci_ResultInvalidLMPParametersInvalidLLParameters;
                action = Ble_LlcpFrameworkActionStop;
            }
            else
            {
                Ble_LlcpConfigureLastScheduledConnEventAfterPassed(pProcedure, pData->pduConnEventCount, pData->instant);
            }

            break;
        }
        default:
        {
            // Should not be reached
            GP_ASSERT_DEV_INT(false);
            action = Ble_LlcpFrameworkActionStop;
            break;
        }
    }

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdatePduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpPhyUpdateData_t * pData;
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodePhyReq:
        {
            if(pContext->masterConnection)
            {
                pProcedure->expectedRxPdu = gpBleLlcp_OpcodePhyRsp;
            }
            else
            {
                pProcedure->expectedRxPdu = gpBleLlcp_OpcodePhyUpdateInd;
            }
            break;
        }
        case gpBleLlcp_OpcodePhyRsp:
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodePhyUpdateInd;
            break;
        }
        case gpBleLlcp_OpcodePhyUpdateInd:
        {
            GP_LOG_PRINTF("Queued PhyUpdateInd tx=%x rx=%x",0, pData->selectedTxPhy, pData->selectedRxPhy);
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;
            if(pData->selectedTxPhy == 0 && pData->selectedRxPhy == 0)
            {
                GP_LOG_PRINTF("send PHY update all zero",0);
                GP_ASSERT_DEV_INT(pData->instant == 0);
                action = Ble_LlcpFrameworkActionStop;
            }
            break;
        }
        default:
        {
            // Should not happen
            GP_ASSERT_DEV_INT(false);
            action = Ble_LlcpFrameworkActionStop;
            break;
        }
    }

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpPhyUpdatePduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    // We need to suspend execution untill the configured instant occurs

    if(txOpcode == gpBleLlcp_OpcodePhyRsp)
    {
        return Ble_LlcpFrameworkActionPause;
    }
    else if(txOpcode == gpBleLlcp_OpcodeRejectExtInd)
    {
        return Ble_LlcpFrameworkActionStop;
    }
    else if(txOpcode == gpBleLlcp_OpcodePhyUpdateInd)
    {
        Ble_LlcpPhyUpdateData_t* pData;
        pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;

        GP_LOG_PRINTF("send phy update ind: %x %x %x",0,pData->instant, pData->txPhys.mask, pData->rxPhys.mask);

        GP_ASSERT_DEV_INT( !HCI_PHYMASK_IS_ZERO(pData->rxPhys) || !HCI_PHYMASK_IS_ZERO(pData->txPhys));
        return Ble_LlcpFrameworkActionPause;
    }

    return Ble_LlcpFrameworkActionContinue;
}

void Ble_LlcpPhyUpdateFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    Ble_LlcpPhyUpdateData_t * pData;
    gpHci_Phy_t newPhyRx;
    gpHci_Phy_t newPhyTx;

    GP_LOG_PRINTF("phy upd finished with res %x",0, pProcedure->result);

    if(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_INTERRUPTED_BY_TERMINATION_IDX))
    {
        Ble_LlcpStopLastScheduledConnEventCount(pContext);
    }

    if(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX))
    {
        return;
    }

    if(pContext->terminationOngoing)
    {
        // Do nothing when we are in the process of terminating the link
        return;
    }

    if(pProcedure->currentTxPdu == gpBleLlcp_OpcodeInvalid && pProcedure->result == gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature)
    {
        // No PDU transmitted, procedure was not started, since the framework detectd it was not supported
        return;
    }

    gpBle_DataTxSetConnectionPause(pContext->connId, false);

    pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;
    newPhyTx = pData->selectedTxPhy;
    newPhyRx = pData->selectedRxPhy;

    if ((pProcedure->localInit && !pProcedure->controllerInit) || (pProcedure->result == gpHci_ResultSuccess &&
        (newPhyTx != gpHci_Phy_None ||
         newPhyRx != gpHci_Phy_None) ) )
    {
        if (pProcedure->result == gpHci_ResultSuccess)
        {
            /* send data length change event if effective octets or timings changed */
            Ble_LlcpPhyUpdateCheckAndScheduleDataLengthChangeEvent(pContext, pProcedure);

            // Update Bandwidth Control: update guard time with new PHY settings
            gpBle_SetConnectionBandwidthControl(pContext->connId, pContext->ccParams.maxCELength, pContext->intervalUnit);
        }

        if ( pProcedure->result != gpHci_ResultSuccess ||
             gpHci_Phy_None == newPhyTx ) /* if phy unchanged */
        {
            newPhyTx = gpBle_GetEffectivePhyTxType(pContext->connId);
            GP_ASSERT_DEV_INT(GP_HCI_PHY_TYPE_VALID(newPhyTx));
        }

        if ( pProcedure->result != gpHci_ResultSuccess ||
             gpHci_Phy_None == newPhyRx ) /* if phy unchanged */
        {
            newPhyRx = gpBle_GetEffectivePhyRxType(pContext->connId);
            GP_ASSERT_DEV_INT(GP_HCI_PHY_TYPE_VALID(newPhyRx));
        }

        if(!notifyHost)
        {
            return;
        }

        Ble_LlcpPhyUpdateScheduleCompleteEvent(pContext->hciHandle, pProcedure->result, newPhyTx, newPhyRx);
    }

    if (pProcedure->result == gpHci_ResultInstantPassed)
    {
        gpSched_ScheduleEventArg(0, Ble_LlcpInstantInvalidFollowUp, pContext);
    }
    else if (pProcedure->result == gpHci_ResultInvalidLMPParametersInvalidLLParameters)
    {
        gpSched_ScheduleEventArg(0, Ble_LlcpParamInvalidFollowUp, pContext);
    }
}

void Ble_LlcpFilterMasterUpdateReqPhys(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    gpHci_PhyMask_t hci_phyNewTxMask;
    gpHci_PhyMask_t hci_phyNewRxMask;
    gpHci_Phy_t phyNewRx;
    gpHci_Phy_t phyNewTx;
    Ble_LlcpPhyUpdateData_t* pData;

    GP_ASSERT_DEV_INT(pContext->masterConnection);
    GP_ASSERT_DEV_INT(pProcedure != NULL);

    pData = pProcedure->pData;

    GP_ASSERT_DEV_INT(pData != NULL);

    GP_LOG_PRINTF("master filter: m: tx=%u rx=%u s: tx=%u rx=%u",0, pData->txPhys.mask, pData->rxPhys.mask, pData->txPhysRemote.mask, pData->rxPhysRemote.mask);

    hci_phyNewTxMask.mask = pData->txPhys.mask & pData->rxPhysRemote.mask;
    hci_phyNewRxMask.mask = pData->rxPhys.mask & pData->txPhysRemote.mask;

    /* If slave wants a symmetric connection it shall set one and the same bit for both rx and tx */
    if(Ble_LlcpIsSymmetricConnectionRequested(pData->txPhysRemote, pData->rxPhysRemote))
    {
        hci_phyNewTxMask.mask &= hci_phyNewRxMask.mask;
        hci_phyNewRxMask.mask = hci_phyNewTxMask.mask;
    }

    phyNewTx = Ble_LlcpPhyUpdateMasterChoosePhy(hci_phyNewTxMask);
    phyNewRx = Ble_LlcpPhyUpdateMasterChoosePhy(hci_phyNewRxMask);

    if (phyNewTx == gpBle_GetEffectivePhyTxType(pContext->connId))
    {
        phyNewTx = gpHci_Phy_None;
    }
    if (phyNewRx == gpBle_GetEffectivePhyRxType(pContext->connId))
    {
        phyNewRx = gpHci_Phy_None;
    }

    pData->selectedTxPhy = phyNewTx;
    pData->selectedRxPhy = phyNewRx;

    GP_LOG_PRINTF("master selected: tx=%u rx=%u",0, pData->selectedTxPhy, pData->selectedRxPhy);
}

void Ble_LlcpParamInvalidFollowUp(void* pArg)
{
    Ble_LlcpLinkContext_t* pContext = (Ble_LlcpLinkContext_t*)pArg;

    GP_LOG_SYSTEM_PRINTF("param invalid => stop",0);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(pContext->connId));

    gpBle_StopConnection(pContext->hciHandle, gpHci_ResultInvalidLMPParametersInvalidLLParameters);
}

gpHci_Phy_t Ble_LlcpPhyUpdateMasterChoosePhy(gpHci_PhyMask_t phys)
{
    if(phys.mask & GP_HCI_PHY_MASK_1MB)
    {
        return gpHci_Phy_1Mb;
    }
#if defined(GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED)
    else if(phys.mask & GP_HCI_PHY_MASK_2MB)
    {
        return gpHci_Phy_2Mb;
    }
#endif
    else
    {
        return gpHci_Phy_None;
    }
}

void Ble_LlcpRegisterPhyUpdateProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdPhyUpdate, &BleLlcpProcedures_PhyUpdateDescriptor);
}


gpHci_Phy_t Ble_LlcpPhyMaskToPhy(gpHci_PhyMask_t phyMask)
{
    UIntLoop i,j,k;

    if(HCI_PHYMASK_IS_ZERO(phyMask))
    {
        return gpHci_Phy_None;
    }

    for(i=0, j=0, k=0; i < 8*sizeof(phyMask.mask); i++)
    {
        if(BIT_TST(phyMask.mask, i))
        {
            k=i;
            j++;
        }
    }
    if(j>1)
    {
        return gpHci_Phy_Invalid;
    }

    if(phyMask.mask != (1 << k))
    {
        return gpHci_Phy_Invalid;
    }

    // The phy type starts counting from 1, but the phy mask starts at bit zero.
    // So if for example bit 2 is set, the result is actually 3.
    return (gpHci_Phy_1Mb + k);
}

Bool Ble_LlcpIsSymmetricConnectionRequested(gpHci_PhyMask_t txPhys, gpHci_PhyMask_t rxPhys)
{
    UIntLoop i;
    UInt8 nrOfBitsSet = 0;

    for(i = 0; i < 8*sizeof(txPhys.mask); i++)
    {
        if(BIT_TST(txPhys.mask, i))
        {
            nrOfBitsSet++;
        }
    }

    // No symmetric connection requested when more than 1 bit set
    if(nrOfBitsSet != 1)
    {
        return false;
    }

    // No symmetric connection requested when tx phys != rx phys
    if(!HCI_PHYMASK_IS_EQUAL(txPhys, rxPhys))
    {
        return false;
    }

    // Both phys are equal and only 1 bit set, symmetric connection
    return true;
}

void Ble_LlcpPhyUpdateScheduleCompleteEvent(gpHci_ConnectionHandle_t connHandle, gpHci_Result_t result, gpHci_Phy_t txPhy, gpHci_Phy_t rxPhy)
{
    gpHci_EventCbPayload_t payload;

    MEMSET(&payload, 0, sizeof(gpHci_EventCbPayload_t));

    payload.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodePhyUpdateComplete;
    payload.metaEventParams.params.phyUpdateComplete.status = result;
    payload.metaEventParams.params.phyUpdateComplete.connectionHandle = connHandle;
    payload.metaEventParams.params.phyUpdateComplete.txPhy = txPhy;
    payload.metaEventParams.params.phyUpdateComplete.rxPhy = rxPhy;

    gpBle_ScheduleEvent(0, gpHci_EventCode_LEMeta, &payload);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_PhyUpdateInit(void)
{
    Ble_LlcpRegisterPhyUpdateProcedure();
}

void Ble_LlcpPhyUpdateCheckAndScheduleDataLengthChangeEvent(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    gpHci_LeMetaDataLengthChange_t params;
    Ble_LlcpPhyUpdateData_t * pData;

    pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;

    if (pData->selectedTxPhy == gpHci_Phy_None && pData->selectedRxPhy == gpHci_Phy_None)
    {
        return;
    }

    params.connectionHandle = pContext->hciHandle;
    params.MaxTxOctets = gpBle_GetEffectiveMaxTxOctets(pContext->connId);
    params.MaxTxTime = gpBle_GetEffectiveMaxTxTime(pContext->connId);
    params.MaxRxOctets = gpBle_GetEffectiveMaxRxOctets(pContext->connId);
    params.MaxRxTime = gpBle_GetEffectiveMaxRxTime(pContext->connId);

    // Send HCI LE Meta event to host if any of the connEffectiveMax parameters changed
    if( pData->effectiveTxOctets != params.MaxTxOctets ||
        pData->effectiveTxTime != params.MaxTxTime     ||
        pData->effectiveRxOctets != params.MaxRxOctets ||
        pData->effectiveRxTime != params.MaxRxTime
      )
    {
        gpHci_EventCbPayload_t payload;
        MEMSET(&payload, 0, sizeof(gpHci_EventCbPayload_t));
        payload.metaEventParams.subEventCode = gpHci_LEMetaSubEventCodeDataLengthChange;
        MEMCPY(&payload.metaEventParams.params.dataLengthChange, &params, sizeof(gpHci_LeMetaDataLengthChange_t));
        gpBle_ScheduleEvent(0, gpHci_EventCode_LEMeta, &payload);

        GP_LOG_PRINTF("DL-PHYUPDATE", 0);
        GP_LOG_PRINTF("prv: %i %i %i %i",0,pData->effectiveTxOctets, pData->effectiveTxTime, pData->effectiveRxOctets, pData->effectiveRxTime);
        GP_LOG_PRINTF("cur: %i %i %i %i",0,params.MaxTxOctets, params.MaxTxTime, params.MaxRxOctets, params.MaxRxTime);
    }
}

void Ble_LlcpPhyUpdateInstantPassed(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    Ble_LlcpPhyUpdateData_t* pData;

    pData = (Ble_LlcpPhyUpdateData_t*)pProcedure->pData;

    GP_ASSERT_DEV_INT(pData);

    //GP_LOG_SYSTEM_PRINTF("LlcpPhyUpdateInstantPassed: tx=%x rx=%x opt=%x",0,pData->selectedTxPhy, pData->selectedRxPhy, pData->phyOptions);

    /* Store previous effective values */
    pData->effectiveTxOctets = gpBle_GetEffectiveMaxTxOctets(pContext->connId);
    pData->effectiveTxTime = gpBle_GetEffectiveMaxTxTime(pContext->connId);
    pData->effectiveRxOctets = gpBle_GetEffectiveMaxRxOctets(pContext->connId);
    pData->effectiveRxTime = gpBle_GetEffectiveMaxRxTime(pContext->connId);

    gpBle_SetEffectivePhys(pContext->connId, pData->selectedTxPhy, pData->selectedRxPhy, pData->phyOptions);

}

void gpBleLlcpProcedures_ControllerTriggeredPhyUpdate(Ble_IntConnId_t connId, gpHci_PhyMask_t txPhys, gpHci_PhyMask_t rxPhys)
{
    gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

    MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
    startDescriptor.procedureId = gpBleLlcp_ProcedureIdPhyUpdate;
    startDescriptor.controllerInit = true;
    startDescriptor.procedureData.phyUpdate.txPhys = txPhys;
    startDescriptor.procedureData.phyUpdate.rxPhys = rxPhys;
    startDescriptor.procedureData.phyUpdate.selectedTxPhy = gpHci_Phy_Invalid;
    startDescriptor.procedureData.phyUpdate.selectedRxPhy = gpHci_Phy_Invalid;

    gpBleLlcpFramework_StartProcedure(connId, &startDescriptor);
}

/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_LeReadPhy(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    Ble_IntConnId_t connId;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    connId = gpBleLlcp_HciHandleToIntHandle(pParams->LeReadPhy.connectionHandle);

    if(!BLE_IS_INT_CONN_HANDLE_VALID(connId))
    {
        return gpHci_ResultUnknownConnectionIdentifier;
    }

    pEventBuf->payload.commandCompleteParams.returnParams.leReadPhy.connectionHandle = pParams->LeReadPhy.connectionHandle;
    pEventBuf->payload.commandCompleteParams.returnParams.leReadPhy.txPhy = gpBle_GetEffectivePhyTxType(connId);
    pEventBuf->payload.commandCompleteParams.returnParams.leReadPhy.rxPhy = gpBle_GetEffectivePhyRxType(connId);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_LeSetDefaultPhy(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;
    gpHci_LeSetDefaultPhyCommand_t* pPhyParams = &pParams->LeSetDefaultPhy;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = Ble_LeSetDefaultPhyParamsChecker(pPhyParams->allPhys, pPhyParams->txPhys, pPhyParams->rxPhys);

    if(result == gpHci_ResultSuccess)
    {
        Ble_LeSetDefaultPhyAction(pPhyParams);
    }

    return result;
}

gpHci_Result_t gpBle_LeSetPhy(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    BLE_SET_RESPONSE_EVENT_COMMAND_STATUS(pEventBuf->eventCode);

    result = Ble_LeSetPhyParamsChecker(&pParams->LeSetPhy);

    if(result == gpHci_ResultSuccess)
    {
        result = Ble_LeSetPhyAction(&pParams->LeSetPhy);
    }

    return result;
}

