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

static Ble_LlcpFrameworkAction_t Ble_LlcpPingQueueingNeeded(Ble_LlcpLinkContext_t* pContext);
static Ble_LlcpFrameworkAction_t Ble_LlcpPingPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpPingPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static void Ble_LlcpRegisterPingProcedure(void);

/*****************************************************************************
 *                    Procedure descriptor
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_PingPduDescriptors[] =
{
    {gpBleLlcp_OpcodePingReq, 0, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH},
    {gpBleLlcp_OpcodePingRsp, 0, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH}
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_PingDescriptor =
{
    .procedureFlags = 0x00,
    .procedureDataLength = 0,
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_PingPduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_PingPduDescriptors,
    // gpBleConfig_FeatureIdLePing exists, but its use is always allowed, so no need to specify here.
    .featureMask = GPBLELLCP_FEATUREMASK_NONE,
    .cbQueueingNeeded = Ble_LlcpPingQueueingNeeded,
    .cbProcedureStart = NULL,
    .cbGetCtrData = NULL,
    .cbStoreCtrData = NULL,
    .cbPduReceived = Ble_LlcpPingPduReceived,
    .cbUnexpectedPduReceived = Ble_LlcpCommonUnexpectedPduReceived,
    .cbPduQueued = Ble_LlcpPingPduQueued,
    .cbPduTransmitted = NULL,
    .cbFinished = NULL
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Ble_LlcpFrameworkAction_t Ble_LlcpPingQueueingNeeded(Ble_LlcpLinkContext_t* pContext)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    if(gpBleLlcpFramework_GetProcedure(pContext, true) != NULL)
    {
        GP_LOG_PRINTF("No need to queue ping, procedure active",0);
        action = Ble_LlcpFrameworkActionStop;
    }

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpPingPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    GP_ASSERT_DEV_INT(pProcedure != NULL);

    switch(rxOpcode)
    {
        case gpBleLlcp_OpcodePingReq:
        {
            pProcedure->currentTxPdu = gpBleLlcp_OpcodePingRsp;
            break;
        }
        case gpBleLlcp_OpcodePingRsp:
        {
            // Receiving ping/unknown response means end of procedure
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

Ble_LlcpFrameworkAction_t Ble_LlcpPingPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodePingReq:
        {
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodePingRsp;
            break;
        }
        case gpBleLlcp_OpcodePingRsp:
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

void Ble_LlcpRegisterPingProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdPing, &BleLlcpProcedures_PingDescriptor);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_PingInit(void)
{
    Ble_LlcpRegisterPingProcedure();
}
