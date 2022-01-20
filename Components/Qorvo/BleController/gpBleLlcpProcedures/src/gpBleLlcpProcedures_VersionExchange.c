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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleLlcpProcedures/src/gpBleLlcpProcedures_VersionExchange.c#1 $
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
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcpProcedures_defs.h"
#include "gpBle_defs.h"
#include "gpLog.h"

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

static Ble_LlcpFrameworkAction_t Ble_LlcpVersionQueueingNeeded(Ble_LlcpLinkContext_t* pContext);
static Ble_LlcpFrameworkAction_t Ble_LlcpVersionStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static Ble_LlcpFrameworkAction_t Ble_LlcpVersionPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpVersionPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static void Ble_LlcpVersionFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpVersionNotifyHost(Ble_LlcpLinkContext_t* pContext, gpHci_Result_t result);
static void Ble_LlcpRegisterVersionProcedure(void);

/*****************************************************************************
 *                    Procedure descriptor
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_VersionExchangePduDescriptors[] =
{
    {gpBleLlcp_OpcodeVersionInd, 5, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH}
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_VersionDescriptor =
{
    .procedureFlags = 0x00,
    .procedureDataLength = 0,
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_VersionExchangePduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_VersionExchangePduDescriptors,
    .featureMask = GPBLELLCP_FEATUREMASK_NONE,
    .cbQueueingNeeded = Ble_LlcpVersionQueueingNeeded,
    .cbProcedureStart = Ble_LlcpVersionStart,
    .cbGetCtrData = Ble_LlcpVersionGetCtrData,
    .cbStoreCtrData = Ble_LlcpVersionStoreCtrData,
    .cbPduReceived = Ble_LlcpVersionPduReceived,
    .cbUnexpectedPduReceived = Ble_LlcpCommonUnexpectedPduReceived,
    .cbPduQueued = Ble_LlcpVersionPduQueued,
    .cbPduTransmitted = NULL,
    .cbFinished = Ble_LlcpVersionFinished
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Ble_LlcpFrameworkAction_t Ble_LlcpVersionQueueingNeeded(Ble_LlcpLinkContext_t* pContext)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    if(pContext->versionExchanged)
    {
        GP_LOG_PRINTF("No need to queue version, info cached",0);

        // Notify host
        Ble_LlcpVersionNotifyHost(pContext, gpHci_ResultSuccess);
        action = Ble_LlcpFrameworkActionStop;
    }

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpVersionStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    GP_ASSERT_DEV_INT(pContext);

    if(pContext->versionExchanged)
    {
        // Procedure should not be started if version was already exchanged
        return Ble_LlcpFrameworkActionStop;
    }

    return Ble_LlcpFrameworkActionContinue;
}

Ble_LlcpFrameworkAction_t Ble_LlcpVersionPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    GP_ASSERT_DEV_INT(!pContext->versionExchanged);

    if(pProcedure->localInit)
    {
        pProcedure->expectedRxPdu = gpBleLlcp_OpcodeVersionInd;
    }
    else
    {
        pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;

        // We can stop if procedure is remote-initiated and we have queued a version PDU
        action = Ble_LlcpFrameworkActionStop;
    }

    // The spec dictates we can only send one VERSION_IND. So, mark this immediately after queueing
    pContext->versionExchanged = true;

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpVersionPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    GP_ASSERT_DEV_INT(pProcedure != NULL);

    if(pContext->versionExchanged)
    {
        // We are only allowed to send 1 VERSION_IND. Stop if version already exchanged
        return Ble_LlcpFrameworkActionStop;
    }

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodeVersionInd:
        {
            pProcedure->currentTxPdu = gpBleLlcp_OpcodeVersionInd;
            break;
        }
        default:
        {
            // Should not happen
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }

    return Ble_LlcpFrameworkActionContinue;
}

void Ble_LlcpVersionFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    if(!notifyHost)
    {
        return;
    }

    if(pProcedure->localInit && !pProcedure->controllerInit)
    {
        Ble_LlcpVersionNotifyHost(pContext, pProcedure->result);
    }
}

void Ble_LlcpVersionNotifyHost(Ble_LlcpLinkContext_t* pContext, gpHci_Result_t result)
{
     // Notify host when needed (when command was issued by the local host)
    gpHci_EventCbPayload_t params;

    params.readRemoteVersionInfoComplete.status = result;
    params.readRemoteVersionInfoComplete.connectionHandle = pContext->hciHandle;
    params.readRemoteVersionInfoComplete.versionNr = pContext->remoteVersionInfo.versionNr;
    params.readRemoteVersionInfoComplete.companyId = pContext->remoteVersionInfo.companyId;
    params.readRemoteVersionInfoComplete.subVersionNr = pContext->remoteVersionInfo.subVersionNr;

    // Needs to be scheduled, because it can be called directly from request function.
    // If not scheduled, the event will be send before the associated command status for the command
    gpBle_ScheduleEvent(0, gpHci_EventCode_ReadRemoteVersionInfoComplete, &params);
}


void Ble_LlcpRegisterVersionProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdVersionExchange, &BleLlcpProcedures_VersionDescriptor);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_VersionExchangeInit(void)
{
    Ble_LlcpRegisterVersionProcedure();
}
