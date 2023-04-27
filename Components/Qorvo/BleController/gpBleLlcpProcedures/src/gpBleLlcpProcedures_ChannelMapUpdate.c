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
#include "gpBleInitiator.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcpProcedures_defs.h"
#include "gpBleLlcpProcedures_Update_defs.h"
#include "gpBle_defs.h"
#include "gpLog.h"
#include "gpSched.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_CTR_DATA_LENGTH_CHAN_MAP_IND 7

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    gpHal_BleChannelMapHandle_t newMasterChannelMapHandle;
    UInt8 newChannelMap[BLE_CHANNEL_MAP_SIZE];
    UInt8 nrOfChannelMapUpdatesPending;
} Ble_ProceduresConnUpdateGlobalContext_t;

// Probably overkill, could be implemented more efficient
typedef struct {
    // this handle is only for slaves
    gpHal_BleChannelMapHandle_t newChannelMapHandle;
} Ble_ProceduresConnUpdateLinkContext_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// Global context (shared for all connections)
static Ble_ProceduresConnUpdateGlobalContext_t Ble_ProceduresConnUpdateGlobalContext;

// Local context (separate for each link)
static Ble_ProceduresConnUpdateLinkContext_t Ble_ProceduresConnUpdateLinkContext[BLE_LLCP_MAX_NR_OF_CONNECTIONS];

/*****************************************************************************
 *                    Static Function Prototypes
*****************************************************************************/

static Ble_LlcpFrameworkAction_t Ble_LlcpChannelMapUpdateStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpChannelMapUpdatePduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
#ifdef GP_DIVERSITY_BLE_PERIPHERAL
static Ble_LlcpFrameworkAction_t Ble_LlcpChannelMapUpdatePduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
#endif // GP_DIVERSITY_BLE_PERIPHERAL
static void Ble_LlcpChannelMapUpdateFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpRegisterChannelMapUpdateProcedure(void);

// various
static void Ble_LlcpFinishChannelMapUpdate(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);

/*****************************************************************************
 *                    Procedure descriptor
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_ChannelMapUpdatePduDescriptors[] =
{
    {gpBleLlcp_OpcodeChannelMapInd, 7, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_CENTRAL},
};

static const  Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_ChannelMapUpdateDescriptor =
{
    .procedureFlags = GPBLELLCPFRAMEWORK_PROCEDURE_FLAGS_INSTANT_BM,
    .procedureDataLength = 0,
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_ChannelMapUpdatePduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_ChannelMapUpdatePduDescriptors,
    .featureMask = GPBLELLCP_FEATUREMASK_NONE,
    .cbQueueingNeeded = NULL,
    .cbProcedureStart = NULL,
    .cbStoreCtrData = Ble_LlcpChannelMapUpdateStoreCtrData,
#ifdef GP_DIVERSITY_BLE_PERIPHERAL
    .cbPduReceived = Ble_LlcpChannelMapUpdatePduReceived,
#endif // GP_DIVERSITY_BLE_PERIPHERAL
    .cbUnexpectedPduReceived = NULL,
    .cbPduQueued = NULL,
    .cbPduTransmitted = Ble_LlcpChannelMapUpdatePduTransmitted,
    .cbFinished = Ble_LlcpChannelMapUpdateFinished
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/


Ble_LlcpFrameworkAction_t Ble_LlcpChannelMapUpdateStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    UInt8 newChannelMap[BLE_CHANNEL_MAP_SIZE];
    UInt16 instant;
    UInt16 pduConnEventCount;
    gpHal_ChannelMap_t halChannelMap;
    UIntLoop i;

    gpPd_ReadWithUpdate(pPdLoh, sizeof(newChannelMap), newChannelMap);
    gpPd_ReadWithUpdate(pPdLoh, sizeof(instant), (UInt8*)&instant);

    // 3 msbs are reserved (cfr Vol 2 Part E $ 7.8.19) - must be ignored on reception
    newChannelMap[BLE_CHANNEL_MAP_SIZE-1] &= 0x1F;

    for(i = 0; i < sizeof(newChannelMap); i++)
    {
        if(newChannelMap[i] != 0)
        {
            break;
        }
    }

    if(i == sizeof(newChannelMap))
    {
        GP_LOG_PRINTF("Received empty channel map",0);
        pProcedure->result = gpHci_ResultInvalidLMPParametersInvalidLLParameters;
        return Ble_LlcpFrameworkActionRejectWithUnknownRsp;
    }

    pduConnEventCount = Ble_LlcpGetPduConnEventCount(pContext, pPdLoh);

    if(!Ble_LlcpInstantValid(instant, pduConnEventCount))
    {
        GP_LOG_PRINTF("Instant is in the past, connection lost",0);
        // Schedule to make sure we do not terminate while procedure still running
        gpSched_ScheduleEventArg(0, Ble_LlcpInstantInvalidFollowUp, pContext);
        return Ble_LlcpFrameworkActionContinue;
    }

    Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle = gpHal_BleAllocateChannelMapHandle();
    GP_ASSERT_DEV_INT(gpHal_BleIsChannelMapValid(Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle));

    Ble_LlcpPopulateChannelRemapTable(&halChannelMap, newChannelMap);

    // Store remap table
    gpHal_BleSetChannelMap(Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle, &halChannelMap);

    Ble_LlcpConfigureLastScheduledConnEventAfterPassed(pProcedure, pduConnEventCount, instant);

    return Ble_LlcpFrameworkActionContinue;
}

Ble_LlcpFrameworkAction_t Ble_LlcpChannelMapUpdatePduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;

    switch(txOpcode)
    {
        case gpBleLlcp_OpcodeChannelMapInd:
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

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpChannelMapUpdatePduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    GP_ASSERT_DEV_INT(pProcedure != NULL);

    pProcedure->currentTxPdu = gpBleLlcp_OpcodeInvalid;
    pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;

    return Ble_LlcpFrameworkActionPause;
}

void Ble_LlcpChannelMapUpdateFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    if(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_INTERRUPTED_BY_TERMINATION_IDX))
    {
        Ble_LlcpStopLastScheduledConnEventCount(pContext);
        Ble_LlcpFinishChannelMapUpdate(pContext, pProcedure);
    }
}
void Ble_LlcpChannelMapUpdatePreInstantPassed(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    gpHal_BleChannelMapHandle_t chanMapHandle;

    if(Ble_LlcpIsMasterConnection(pContext->connId))
    {
        chanMapHandle = Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle;
    }
    else
    {
        chanMapHandle = Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle;
    }

    gpHal_BleUpdateChannelMap(pContext->connId, chanMapHandle);

    pProcedure->result = gpHci_ResultSuccess;

    Ble_LlcpFinishChannelMapUpdate(pContext, pProcedure);
}

void Ble_LlcpRegisterChannelMapUpdateProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdChannelMapUpdate, &BleLlcpProcedures_ChannelMapUpdateDescriptor);

    gpBleLlcpFramework_RegisterInvalidProcedureAction(gpBleLlcp_ProcedureIdChannelMapUpdate, false);
}

void Ble_LlcpFinishChannelMapUpdate(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{

    if(pProcedure->localInit)
    {
        // Master case
        Ble_ProceduresConnUpdateGlobalContext.nrOfChannelMapUpdatesPending--;

        if(Ble_ProceduresConnUpdateGlobalContext.nrOfChannelMapUpdatesPending == 0)
        {
            // All links are using new channel map ==> old can be freed
            Ble_LlcpUpdateMasterChannelMapHandle(Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle);
            Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;
        }
    }
    else
    {
        // Slave case
        if(Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle != GP_HAL_BLE_CHANNEL_MAP_INVALID)
        {
            // First free old handle, afterwards update handle in context (if procedure was success)
            GP_ASSERT_DEV_INT(gpHal_BleIsChannelMapValid(Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle));

            if(pProcedure->result == gpHci_ResultSuccess)
            {
                // Success, free old channel map and apply new
                gpHal_BleFreeChannelMapHandle(pContext->channelMapHandle);
                pContext->channelMapHandle = Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle;
            }
            else
            {
                // Failure, free the new channel map (old will be freed during termination of connection)
                gpHal_BleFreeChannelMapHandle(Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle);
            }
            Ble_ProceduresConnUpdateLinkContext[pContext->connId].newChannelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;
        }
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/




void gpBleLlcpProcedures_ChannelMapUpdateInit(void)
{
    Ble_LlcpRegisterChannelMapUpdateProcedure();
}

void gpBleLlcpProcedures_ChannelMapUpdateReset(Bool firstReset)
{
    UIntLoop i;


    if(firstReset)
    {
        // Make sure it is initialized to an invalid value, will be checked in reset!
        Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;

        // Reset link context
        for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
        {
            Ble_ProceduresConnUpdateLinkContext[i].newChannelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;
        }
    }
    else
    {
        // Reset global context
        Ble_ProceduresConnUpdateGlobalContext.nrOfChannelMapUpdatesPending = 0;

        if(Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle != GP_HAL_BLE_CHANNEL_MAP_INVALID)
        {
            gpHal_BleFreeChannelMapHandle(Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle);
            Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;
        }

        // Reset link context
        for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
        {
            // release new/temporary channel map handles that are still in use on some (possibly multiple ?) connections
            if(Ble_ProceduresConnUpdateLinkContext[i].newChannelMapHandle != GP_HAL_BLE_CHANNEL_MAP_INVALID)
            {
                if (gpHal_BleIsChannelMapValid(Ble_ProceduresConnUpdateLinkContext[i].newChannelMapHandle))
                {
                    gpHal_BleFreeChannelMapHandle(Ble_ProceduresConnUpdateLinkContext[i].newChannelMapHandle);
                }
                Ble_ProceduresConnUpdateLinkContext[i].newChannelMapHandle = GP_HAL_BLE_CHANNEL_MAP_INVALID;
            }
        }
    }
}

gpHal_BleChannelMapHandle_t Ble_LlcpGetLatestChannelMapHandle(gpHal_BleChannelMapHandle_t masterChannelMapHandle)
{
    if (GP_HAL_BLE_CHANNEL_MAP_INVALID == Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle)
    {
        return masterChannelMapHandle;
    }
    else
    {
        return Ble_ProceduresConnUpdateGlobalContext.newMasterChannelMapHandle;
    }
}

/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/


