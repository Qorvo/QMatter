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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleLlcpProcedures/src/gpBleLlcpProcedures_AclTermination.c#1 $
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
#include "gpSched.h"
#include "gpBleDataTx.h"

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

static Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpAclTerminationGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* ctrDataLength, UInt8* pCtrData);
static Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationPduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static void Ble_LlcpAclTerminationFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static void Ble_LlcpRegisterAclTerminationProcedure(void);

// Timeout functions
static void Ble_LlcpAclTerminationTimeout(void* pArgs);
static void Ble_LlcpAclTerminationSuccess(void* pArgs);
static void Ble_LlcpAclTerminationCommon(Ble_LlcpProcedureContext_t* pProcedure);

/*****************************************************************************
 *                    Procedure descriptor
*****************************************************************************/

static const gpBleLlcpFramework_PduDescriptor_t BleLlcpProcedures_AclTerminationPduDescriptors[] =
{
    {gpBleLlcp_OpcodeTerminateInd, 1, GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_BOTH}
};

static const Ble_LlcpProcedureDescriptor_t BleLlcpProcedures_AclTerminationDescriptor =
{
    .procedureFlags = 0x00,
    .procedureDataLength = sizeof(gpBleLlcpProcedureTerminationData_t),
    .nrOfPduDescriptors = number_of_elements(BleLlcpProcedures_AclTerminationPduDescriptors),
    .pPduDescriptors = BleLlcpProcedures_AclTerminationPduDescriptors,
    .featureMask = GPBLELLCP_FEATUREMASK_NONE,
    .cbQueueingNeeded = NULL,
    .cbProcedureStart = Ble_LlcpAclTerminationStart,
    .cbGetCtrData = Ble_LlcpAclTerminationGetCtrData,
    .cbStoreCtrData = Ble_LlcpAclTerminationStoreCtrData,
    .cbPduReceived = Ble_LlcpAclTerminationPduReceived,
    .cbUnexpectedPduReceived = NULL,
    .cbPduQueued = Ble_LlcpAclTerminationPduQueued,
    .cbPduTransmitted = Ble_LlcpAclTerminationPduTransmitted,
    .cbFinished = Ble_LlcpAclTerminationFinished
};

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationStart(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    if(pContext->terminationOngoing && pProcedure->localInit)
    {
        // Stop immediately when termination is already ongoing
        return Ble_LlcpFrameworkActionStop;
    }

    if(!pContext->masterConnection)
    {
        // Make sure local procedures are allowed to run (they could have been disabled by a remote init encryption procedure)
        gpBleLlcpFramework_EnableProcedureHandling(pContext, true, true);
    }

    pContext->terminationOngoing = true;
    gpBle_DataTxSetConnectionPause(pContext->connId, true);

    // Maximize the connection event duration (this will disable controlled bandwidth) and make sure we are not
    // stopping our connection events too early (and reduce the risk to miss an acknowledgement on our LL_TERMINATE_IND PDU).
    pContext->ccParams.maxCELength = BLE_CE_LENGTH_MAX;
    gpBle_SetConnectionBandwidthControl(pContext->connId, pContext->ccParams.maxCELength, pContext->intervalUnit);

    // Update connection priority for termination stage
    gpHal_SetConnectionPriority(pContext->connId, Ble_Priority_High);

    return Ble_LlcpFrameworkActionContinue;
}

void Ble_LlcpAclTerminationGetCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* ctrDataLength, UInt8* pCtrData)
{
    GP_ASSERT_DEV_INT(pProcedure->localInit);

    GP_LOG_PRINTF("Termination get ctrData %x",0, *pOpcode);

    switch(*pOpcode)
    {
        case gpBleLlcp_OpcodeTerminateInd:
        {
            UInt8* pReason = (UInt8*)pProcedure->pData;

             MEMCPY(&pCtrData[0], pReason, 1);
            *ctrDataLength = 1;
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

Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationStoreCtrData(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    GP_ASSERT_DEV_INT(!pProcedure->localInit);

    GP_LOG_PRINTF("Termination store ctrData %x",0, opcode);

    switch(opcode)
    {
        case gpBleLlcp_OpcodeTerminateInd:
        {
            gpBleLlcpProcedureTerminationData_t* pTerm = (gpBleLlcpProcedureTerminationData_t*)pProcedure->pData;
            pTerm->reason = gpPd_ReadByte(pPdLoh->handle, pPdLoh->offset);
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

Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationPduQueued(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    GP_ASSERT_DEV_INT(pProcedure->localInit);

    GP_LOG_PRINTF("Termination pdu queued %x",0, txOpcode);

    pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;

    // TODO: let connection monitor handle this timeout
    gpSched_ScheduleEventArg(BLE_TIME_UNIT_10000_TO_US(pContext->timeoutUnit), Ble_LlcpAclTerminationTimeout, pProcedure);

    return Ble_LlcpFrameworkActionPause;
}

Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationPduReceived(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    UInt32 ackRetryPeriod;

    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(!pProcedure->localInit);

    GP_LOG_PRINTF("Termination pdu received %x",0, rxOpcode);

    if(pContext->masterConnection)
    {
        /* If TERMINATE_IND was received from slave the master will only send an ACK at the next connection event.
         * This means we need to delay the closing of the connection for 1 interval (SDB004-555).
         */
        ackRetryPeriod = BLE_TIME_UNIT_1250_TO_US(pContext->intervalUnit);
    }
    else
    {
        /* If TERMINATE_IND is sent from master, the slave shall ACK immediatly, instead of waiting for next connection event.
         * ACKs might still be lost but we don't worry about this and rely on timeout.
         */
        ackRetryPeriod = 0;
    }

    gpSched_ScheduleEventArg(ackRetryPeriod, Ble_LlcpAclTerminationSuccess, pProcedure);

    return Ble_LlcpFrameworkActionPause;
}

Ble_LlcpFrameworkAction_t Ble_LlcpAclTerminationPduTransmitted(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    GP_ASSERT_DEV_INT(pProcedure->localInit);

    GP_LOG_PRINTF("Termination pdu transmitted %x",0, txOpcode);

    // TODO: replace with call to connection monitor
    if (gpSched_UnscheduleEventArg(Ble_LlcpAclTerminationTimeout, pProcedure))
    {
        Ble_LlcpAclTerminationSuccess(pProcedure);
    }

    return Ble_LlcpFrameworkActionPause;
}

void Ble_LlcpAclTerminationFinished(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    // There could have been overlap between the local and remote initiated disconnect procedure.
    // Make sure to unschedule any termination function still pending
    gpSched_UnscheduleEventArg(Ble_LlcpAclTerminationTimeout, pProcedure);
    gpSched_UnscheduleEventArg(Ble_LlcpAclTerminationSuccess, pProcedure);
    GP_LOG_PRINTF("Term finished: proc: %lx en id: %x",0,(unsigned long int)pProcedure, pContext->connId);
}

void Ble_LlcpRegisterAclTerminationProcedure(void)
{
    gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureIdTermination, &BleLlcpProcedures_AclTerminationDescriptor);
}

void Ble_LlcpAclTerminationTimeout(void* pArgs)
{
    Ble_LlcpProcedureContext_t* pProcedure = (Ble_LlcpProcedureContext_t*)pArgs;

    GP_LOG_SYSTEM_PRINTF("Termination timeout!",0);

    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(Ble_LlcpIsConnectionAllocated(pProcedure->connId));

    Ble_LlcpAclTerminationCommon(pProcedure);
}

void Ble_LlcpAclTerminationSuccess(void* pArgs)
{
    Ble_LlcpProcedureContext_t* pProcedure = (Ble_LlcpProcedureContext_t*)pArgs;

    GP_LOG_PRINTF("Term Success",0);

    // Restore default priority
    gpHal_SetConnectionPriority(pProcedure->connId, BLE_CONN_PRIORITY);

    Ble_LlcpAclTerminationCommon(pProcedure);
}

void Ble_LlcpAclTerminationCommon(Ble_LlcpProcedureContext_t* pProcedure)
{
    UInt8* pReason;

    if(pProcedure == NULL || !Ble_LlcpIsConnectionAllocated(pProcedure->connId))
    {
        // This can happen when the connection was already terminated (e.g. overlap of termination procedures)
        GP_LOG_PRINTF("No valid link context ==> drop trigger",0);
        return;
    }

    pReason = (UInt8*)pProcedure->pData;

    if(pProcedure->localInit)
    {
        *pReason = gpHci_ResultConnectionTerminatedByLocalHost;
    }

    gpBle_StopConnection(gpBleLlcp_IntHandleToHciHandle(pProcedure->connId), *pReason);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpProcedures_AclTerminationInit(void)
{
    Ble_LlcpRegisterAclTerminationProcedure();
}

void gpBleLlcpProcedures_AclTerminationReset(void)
{
    // NULL is a wildcard for all arguments
    gpSched_UnscheduleEventArg(Ble_LlcpAclTerminationTimeout, NULL);
    gpSched_UnscheduleEventArg(Ble_LlcpAclTerminationSuccess, NULL);
}

