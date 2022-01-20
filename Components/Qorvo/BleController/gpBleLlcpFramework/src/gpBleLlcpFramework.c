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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleLlcpFramework/src/gpBleLlcpFramework.c#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */

// #define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLELLCPFRAMEWORK

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleDataCommon.h"
#include "gpBleDataChannelTxQueue.h"
#include "gpBle_defs.h"
#include "gpBleLlcp.h"
#include "gpBleLlcpFramework.h"
#include "gpBleLlcpProcedures.h"
#include "gpSched.h"
#include "gpPoolMem.h"
#include "gpLog.h"
#include "gpHci.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_LLCP_PROCEDURE_RESPONSE_TIMEOUT_MS       40000

#ifndef GP_BLE_NR_OF_SUPPORTED_PROCEDURES
#error GP_BLE_NR_OF_SUPPORTED_PROCEDURES should have been defined!
#endif

#ifndef GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS
#error GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS should have been defined!
#endif

#if GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS == 0
#undef GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS
#define GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS 1
#endif

// Specifies how many procedure are enabled
#define BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES     (GP_BLE_NR_OF_SUPPORTED_PROCEDURES)

#define BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION     5
#define BLE_LLCP_MAX_NR_OF_INVALID_PROCEDURE_ROLE_COMBINATIONS  7

#define BLE_LLCP_MAX_NR_OF_SIMULTANEOUS_ACTIVE_PROCEDURES       2

#define BLE_CONTROL_PDU_OPCODE_OFFSET       0
#define BLE_CONTROL_PDU_CTR_DATA_OFFSET     1

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_IS_LLCP_PROCEDURE_VALID(procedureId)                (procedureId < gpBleLlcp_ProcedureIdInvalid)

#define BLE_LLCP_QUEUE_PTR_INCREMENT(ptr)       (((ptr + 1) < BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION) ? (ptr + 1) : 0)

#ifdef GP_LOCAL_LOG
#define BLE_PRINT_LLCP_OPCODE(x) PrintLLCP_Opcode(x)
#else
#define BLE_PRINT_LLCP_OPCODE(x) do {} while(false)
#endif

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

// Description of the procedure queue for a connection
typedef struct {
    Bool active;
    UInt8 queueRdPointer;
    UInt8 queueWrPointer;
    UInt8 nrOfQueuedEntries;
    Ble_LlcpProcedureContext_t* queuedProcedures[BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION];
    Ble_LlcpProcedureContext_t* pRemoteProcedure;
} Ble_LlcpProcedureQueue_t;

// Description of an invalid (not allowed) procedure and role (master/slave) combination
typedef struct {
    gpBleLlcp_ProcedureId_t procedureId;
    Bool masterRole;
} Ble_LlcpInvalidProcedureAction_t;

// Description of an invalid (not allowed) opcode and role (master/slave) combination
typedef struct {
    gpBleLlcp_Opcode_t opcode;
    Bool masterRole;
} Ble_LlcpInvalidPduAction_t;

typedef struct {
    gpBleLlcp_ProcedureId_t procedureId;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;
} Ble_LlcpRegisteredProcedure_t;

typedef struct {
    gpBleLlcp_ProcedureId_t procedureId;
    const gpBleLlcpFramework_ProcedureUserCallbacks_t* pCallbacks;
} Ble_LlcpRegisteredProcedureCallback_t;

// Description of the global framework context
typedef struct {
    Ble_LlcpRegisteredProcedure_t procedures[BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES];
    Ble_LlcpRegisteredProcedureCallback_t callbacks[GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS];
    Ble_LlcpInvalidProcedureAction_t invalidProcedureActions[BLE_LLCP_MAX_NR_OF_INVALID_PROCEDURE_ROLE_COMBINATIONS];
} Ble_LlcpFrameworkContext_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// LLCP Framework global context
static Ble_LlcpFrameworkContext_t Ble_FrameworkGlobalContext;

// LLCP Framework link context per connection
static Ble_LlcpProcedureQueue_t Ble_LlcpProcedureQueue[BLE_LLCP_MAX_NR_OF_CONNECTIONS];

#ifdef GP_LOCAL_LOG
const static char* Ble_LlcpOpcodeStrings[] =
{
    " LL_CONNECTION_UPDATE_IND",    /* 0x00 */
    " LL_CHANNEL_MAP_IND",          /* 0x01 */
    " LL_TERMINATE_IND",            /* 0x02 */
    " LL_ENC_REQ",                  /* 0x03 */
    " LL_ENC_RSP",                  /* 0x04 */
    " LL_START_ENC_REQ",            /* 0x05 */
    " LL_START_ENC_RSP",            /* 0x06 */
    " LL_UNKNOWN_RSP",              /* 0x07 */
    " LL_FEATURE_REQ",              /* 0x08 */
    " LL_FEATURE_RSP",              /* 0x09 */
    " LL_PAUSE_ENC_REQ",            /* 0x0A */
    " LL_PAUSE_ENC_RSP",            /* 0x0B */
    " LL_VERSION_IND",              /* 0x0C */
    " LL_REJECT_IND",               /* 0x0D */
    " LL_SLAVE_FEATURE_REQ",        /* 0x0E */
    " LL_CONNECTION_PARAM_REQ",     /* 0x0F */
    " LL_CONNECTION_PARAM_RSP",     /* 0x10 */
    " LL_REJECT_EXT_IND",           /* 0x11 */
    " LL_PING_REQ",                 /* 0x12 */
    " LL_PING_RSP",                 /* 0x13 */
    " LL_LENGTH_REQ",               /* 0x14 */
    " LL_LENGTH_RSP",               /* 0x15 */
    " LL_PHY_REQ",                  /* 0x16 */
    " LL_PHY_RSP",                  /* 0x17 */
    " LL_PHY_UPDATE_IND",           /* 0x18 */
    " LL_MIN_USED_CHANNELS_IND",    /* 0x19 */
    " LL_CTE_REQ",                  /* 0x1A */
    " LL_CTE_RSP",                  /* 0x1B */
    " LL_PERIODIC_SYNC_IND",        /* 0x1C */
    " LL_CLOCK_ACCURACY_REQ",       /* 0x1D */
    " LL_CLOCK_ACCURACY_RSP",       /* 0x1E */
    " LL_CIS_REQ",                  /* 0x1D */
    " LL_CIS_RSP",                  /* 0x20 */
    " LL_CIS_IND",                  /* 0x21 */
    " LL_CIS_TERMINATE_IND",        /* 0x22 */
    " LL_POWER_CONTROL_REQ",        /* 0x23 */
    " LL_POWER_CONTROL_RSP",        /* 0x24 */
    " LL_POWER_CHANGE_IND",         /* 0x25 */
};
#endif //GP_LOCAL_LOG

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

// Reset
static void Ble_LlcpResetProcedureQueue(Ble_LlcpLinkContext_t* pContext, Bool fromHciReset);
static void Ble_LlcpFrameworkResetSchedule(Ble_LlcpLinkContext_t* pContext);
static void BleLlcpFramework_CloseConnectionInternal(Ble_IntConnId_t connId, Bool fromHciReset);

// Procedure start
static Ble_LlcpProcedureContext_t* Ble_LlcpAllocateProcedure(Ble_LlcpLinkContext_t* pContext, Bool localInit, Bool controllerInit, gpBleLlcp_ProcedureId_t procedureId);
static Bool Ble_LlcpIsProcedureStartAllowed(Ble_LlcpProcedureContext_t* pProcedure, UInt8 queueIndex);
static Bool Ble_LlcpIsProcedureAlwaysAllowed(gpBleLlcp_ProcedureId_t procedureId);
static Bool Ble_LlcpIsProcedureSupportedOnLink(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_ProcedureId_t procedureId);
static Bool Ble_LlcpProcedureSchedulingAllowed(gpBleLlcp_ProcedureId_t procedureId, Bool masterRole);
static Ble_LlcpProcedureContext_t* Ble_LlcpGetFirstAllowedProcedure(Ble_LlcpLinkContext_t* pContext);
static Ble_LlcpProcedureContext_t* Ble_LlcpSelectNextLocalProcedure(Ble_LlcpLinkContext_t* pContext);
static void Ble_LlcpTriggerProcedureQueue(void* pArgs);
static void Ble_LlcpStartProcedureLocal(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpStartProcedureRemote(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static void Ble_LlcpStartProcedureCommon(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpFrameworkAction_t action);

// Procedure timeout/cleanup
static void Ble_LlcpFreeProcedure(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpFreeProcedureData(Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpFrameworkFreeProcedureAndData(Ble_LlcpProcedureContext_t* pProcedure);
static void Ble_LlcpFrameworkFreeProcedureAndDataDelayed(void* pArgs);
static void Ble_LlcpCleanupProcedure(Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpLinkContext_t* pContext, Bool removePrstUser);
static void Ble_LlcpAddPrstUser(Ble_LlcpLinkContext_t* pContext);
static void Ble_LlcpEnablePrst(Ble_LlcpLinkContext_t* pContext, Bool enable);
static void Ble_LlcpRemovePrstUser(Ble_LlcpLinkContext_t* pContext);
static void Ble_LlcpPrstExpired(void* pArg);

// Procedure handling TX
static void Ble_LlcpSendPduIfResourceAvailable(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t opcode, UInt8 dataLength, UInt8* pData);
static void Ble_LlcpTrySendingPdu(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpPduResource_t* pResource);
static void Ble_LlcpGetTxResource(gpPd_Loh_t* pPdLoh, Ble_LlcpLinkContext_t* pContext);
static void Ble_LlcpSendReject(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t opcode, gpHci_Result_t reason);
static void Ble_LlcpSendUnknownRsp(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t opcode);
static Bool Ble_LlcpPduTransmitted(Ble_LlcpProcedureContext_t* pProcedure, gpPd_Handle_t pdHandle);
static void Ble_LlcpEmptyQueueCb(Ble_IntConnId_t connId);
static Bool Ble_LlcpIsEmptyQueueCallbackPending(Ble_LlcpLinkContext_t* pContext);

// Procedure handling RX
static Bool Ble_LlcpProcessIncomingPdu(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode, gpPd_Loh_t pdLoh);
static gpBleLlcp_ProcedureId_t Ble_LlcpGetProcedureIdFromStartPdu(gpBleLlcp_Opcode_t rxLlcpOpcode);
static Bool Ble_LlcpFollowupCommon(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh, Bool* pFreePd);
static Bool Ble_LlcpFollowupLocal(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh, Bool* pFreePd);
static Bool Ble_LlcpFollowupRemote(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh, Bool* pFreePd);
static Bool Ble_LlcpFollowupNewRemote(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh, Bool* pFreePd);
static Bool Ble_LlcpPduReceived(Ble_LlcpLinkContext_t* pContext, gpPd_Loh_t pdLoh);
static void Ble_LlcpProcessQueuedPdu(void* pArg);

// Check PDU validity
static gpHci_Result_t Ble_LlcpGetCollisionResult(gpBleLlcp_ProcedureId_t localProcedureId, gpBleLlcp_ProcedureId_t remoteProcedureId);
static gpHci_Result_t Ble_LlcpProcedureNoCollisionDetected(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode);
static gpHci_Result_t Ble_LlcpProcedurePduAllowed(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode, UInt8 allowedRoleMask);
static gpHci_Result_t Ble_LlcpAcceptIncomingPdu(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode);
static Bool Ble_LlcpIsValidPdu(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode);
static Bool Ble_LlcpIsCtrDataLengthValid(gpBleLlcp_Opcode_t opcode, UInt8 ctrDataLength);

// Callbacks to procedures
static inline Ble_LlcpFrameworkAction_t Ble_LlcpTriggerQueueingNeededIfRegistered(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_ProcedureId_t procedureId);
static inline Ble_LlcpFrameworkAction_t Ble_LlcpTriggerStartIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static inline void Ble_LlcpTriggerGetCtrDataIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData);
static inline Ble_LlcpFrameworkAction_t Ble_LlcpTriggerStoreCtrDataIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode);
static inline Ble_LlcpFrameworkAction_t Ble_LlcpTriggerPduQueuedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static inline Ble_LlcpFrameworkAction_t Ble_LlcpTriggerPduTxedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode);
static inline Ble_LlcpFrameworkAction_t Ble_LlcpTriggerPduReceivedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode);
static inline Ble_LlcpFrameworkAction_t Ble_LlcpTriggerUnexpectedPduReceivedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh);
static inline void Ble_LlcpTriggerFinishedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost);
static inline void Ble_LlcpTriggerUserCbIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool start);

// Procedure descriptor handling
static const Ble_LlcpProcedureDescriptor_t* BleLlcpFramework_GetProcedureDescriptor(gpBleLlcp_ProcedureId_t procedureId);

// Helpers
static inline Bool Ble_LlcpIsLocalInitiatedProcedure(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static Bool Ble_LlcpIsInstantProcedure(gpBleLlcp_ProcedureId_t procedureId);
static Bool Ble_LlcpCommonIsRejectPdu(gpBleLlcp_Opcode_t opcode);
static Bool Ble_LlcpIsExtendedRejectSupported(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure);
static UInt8 Ble_LlcpRoleToRoleMask(Bool masterTransmitter);
static gpBleLlcp_Opcode_t Ble_LlcpGetStartPduFromProcedureDescriptor(const Ble_LlcpProcedureDescriptor_t* pDescriptor);
static Bool Ble_LlcpProcedureHasValidPduDescriptors(const Ble_LlcpProcedureDescriptor_t* pDescriptor);

#ifdef GP_LOCAL_LOG
static void PrintLLCP_Opcode(gpBleLlcp_Opcode_t opcode);
#endif // GP_LOCAL_LOG

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void Ble_LlcpResetProcedureQueue(Ble_LlcpLinkContext_t* pContext, Bool fromHciReset)
{
    UIntLoop i;
    Ble_LlcpProcedureContext_t* pProcedures[] = {pContext->pProcedureLocal, pContext->pProcedureRemote};

    // First cleanup any active local and remote procedure
    for(i = 0; i < number_of_elements(pProcedures); i++)
    {
        if(pProcedures[i] != NULL)
        {
            if(!GP_HCI_RESULT_VALID(pProcedures[i]->result))
            {
                // Only modify result if not done already!
                pProcedures[i]->result = gpHci_ResultDifferentTransactionCollision;
                gpBleLlcpFramework_ProcedureStateSet(pProcedures[i], BLE_LLCP_PROCEDURE_INTERRUPTED_BY_TERMINATION_IDX);
            }
            // No need to remove PRST, queue is reset, and this will be handled automatically
            Ble_LlcpCleanupProcedure(pProcedures[i], pContext, false);
        }
    }

    // Cleanup queued (but not started) procedures
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION; i++)
    {
        if(Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i] != NULL)
        {
            if(fromHciReset)
            {
                // When an HCI reset is performed, the link context will be freed before any ongoing termination.
                // Unschedule these here, and rely on the direct free below
                if(gpSched_ExistsEventArg(Ble_LlcpFrameworkFreeProcedureAndDataDelayed, (void*)Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i]))
                {
                    gpSched_UnscheduleEventArg(Ble_LlcpFrameworkFreeProcedureAndDataDelayed, (void*)Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i]);
                }
            }

            /* do not free a procedure here if cleanup has already been scheduled for it */
            if (!gpSched_ExistsEventArg(Ble_LlcpFrameworkFreeProcedureAndDataDelayed, (void*)Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i]))
            {
                Ble_LlcpFreeProcedureData(Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i]);
                gpPoolMem_Free(Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i]);
                Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i] = NULL;
            }
        }
    }

    // Cleanup framework context
    Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries = 0;
    Ble_LlcpProcedureQueue[pContext->connId].queueRdPointer = 0;
    Ble_LlcpProcedureQueue[pContext->connId].queueWrPointer = 0;
    Ble_LlcpProcedureQueue[pContext->connId].active = false;
}

// This function removes all scheduled triggers for a connection.
void Ble_LlcpFrameworkResetSchedule(Ble_LlcpLinkContext_t* pContext)
{
    // Unschedule functions
    Ble_LlcpEnablePrst(pContext, false);
    while(gpSched_UnscheduleEventArg(Ble_LlcpTriggerProcedureQueue, pContext));
    while(gpSched_UnscheduleEventArg(Ble_LlcpProcessQueuedPdu, pContext));
}

void BleLlcpFramework_CloseConnectionInternal(Ble_IntConnId_t connId, Bool fromHciReset)
{
    Ble_LlcpLinkContext_t* pContext = Ble_GetLinkContext(connId);

    GP_LOG_PRINTF("LlcpFramework close conn %u",0, connId);

    if(pContext == NULL || !BLE_IS_INT_CONN_HANDLE_VALID(pContext->connId))
    {
        // No valid context, drop request
        return;
    }

    // Delete all procedures
    Ble_LlcpResetProcedureQueue(pContext, fromHciReset);

    // Make sure to remove any pending framework triggers for this connection
    Ble_LlcpFrameworkResetSchedule(pContext);

    // Procedure specific cleanup
    Ble_LlcpProcedureUpdateResetConnection(pContext);
}

// localInit: indicates whenever this procedure is initiated by this side of the connection
// controllerInit: indicates whenever it was the host or the controller for this side of the connection that initiated the procedure
Ble_LlcpProcedureContext_t* Ble_LlcpAllocateProcedure(Ble_LlcpLinkContext_t* pContext, Bool localInit, Bool controllerInit, gpBleLlcp_ProcedureId_t procedureId)
{
    UIntLoop i;
    UInt8 wrPointer;

    if(!BLE_IS_LLCP_PROCEDURE_VALID(procedureId))
    {
        return NULL;
    }

    wrPointer = Ble_LlcpProcedureQueue[pContext->connId].queueWrPointer;
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION; i++)
    {
        Ble_LlcpProcedureContext_t* pProcedure = Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[wrPointer];

        if(Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries == BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION)
        {
            // Queue for this connection is full
            //gpHci_stopCommands(); <-- to be enabled
            GP_LOG_PRINTF("Connection queue is full",0);
            return NULL;
        }

        if(pProcedure == NULL)
        {
            // We have an available entry
            pProcedure = (Ble_LlcpProcedureContext_t*)GP_POOLMEM_MALLOC(sizeof(Ble_LlcpProcedureContext_t));

            pProcedure->connId = pContext->connId;
            pProcedure->controllerInit= controllerInit;
            pProcedure->localInit = localInit;
            pProcedure->procedureId = procedureId;
            pProcedure->state = 0x0;
            pProcedure->result = gpHci_ResultInvalid;
            pProcedure->pData = NULL;
            pProcedure->dataLength = 0;
            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;
            pProcedure->currentTxPdu = gpBleLlcp_OpcodeInvalid;
            pProcedure->pdHandle = GP_PD_INVALID_HANDLE;

            if(localInit)
            {
                Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[wrPointer] = pProcedure;

                if (0 == Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries)
                {
                    gpBleLlcp_ProhibitSlaveLatency(pContext->connId, true, Ble_ProhibitSlaveLatency_LocalProcedure);
                }
                // Update queue administration
                Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries++;
                Ble_LlcpProcedureQueue[pContext->connId].queueWrPointer = BLE_LLCP_QUEUE_PTR_INCREMENT(Ble_LlcpProcedureQueue[pContext->connId].queueWrPointer);

                // Trigger procedure queue if needed
                if(!gpSched_ExistsEventArg(Ble_LlcpTriggerProcedureQueue, pContext))
                {
                    // Make sure to use exist instead of unschedule (keeps current entry at his position)
                    gpSched_ScheduleEventArg(0, Ble_LlcpTriggerProcedureQueue, pContext);
                }
            }
            else
            {
                Ble_LlcpProcedureQueue[pContext->connId].pRemoteProcedure = pProcedure;
                gpBleLlcp_ProhibitSlaveLatency(pContext->connId, true, Ble_ProhibitSlaveLatency_RemoteProcedure);
            }

            GP_LOG_PRINTF("added proc: %x en nr: %x",0,procedureId, Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries);
            return pProcedure;
        }
        wrPointer =  BLE_LLCP_QUEUE_PTR_INCREMENT(wrPointer);
    }

    GP_LOG_PRINTF("Nothing to add",0);
    return NULL;
}

Bool Ble_LlcpIsProcedureStartAllowed(Ble_LlcpProcedureContext_t* pProcedure, UInt8 queueIndex)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(pProcedure->connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    if(pContext->pProcedureLocal == NULL && pContext->pProcedureRemote == NULL)
    {
        // No procedure currently running, procedure start is allowed.
        return true;
    }

    if(queueIndex == 0 && pContext->pProcedureLocal == NULL)
    {
        // There is only a remote procedure running.
        // Procedure start for first queue entry is only allowed when this and the remote procedure are not both instant procedures
        return !(Ble_LlcpIsInstantProcedure(pProcedure->procedureId) && Ble_LlcpIsInstantProcedure(pContext->pProcedureRemote->procedureId));
    }
    else
    {
        // We are trying to start a procedure that is not in front of the queue OR there is alread a local procedure ongoing.
        // Only "always allowed" procedures can be started in this case.
        return Ble_LlcpIsProcedureAlwaysAllowed(pProcedure->procedureId);
    }
}

Bool Ble_LlcpIsProcedureAlwaysAllowed(gpBleLlcp_ProcedureId_t procedureId)
{
    if(procedureId == gpBleLlcp_ProcedureIdTermination)
    {
        return true;
    }

    return false;
}

Bool Ble_LlcpIsProcedureSupportedOnLink(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_ProcedureId_t procedureId)
{
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(procedureId);

    if(pContext->featuresExchangedStatus != gpBleLlcp_FeatureStatus_ExchangedSuccess && GPBLELLCPFRAMEWORK_PROCEDURE_FLAGS_LONG_PDUS_GET(pDescriptor->procedureFlags))
    {
        return false;
    }

    if(pDescriptor->featureMask == GPBLELLCP_FEATUREMASK_NONE)
    {
        // No feature mask specified, procedure is supported by default
        return true;
    }
    else
    {
        if((pContext->allowedProcedures & pDescriptor->featureMask) != 0)
        {
            // There is a match in features, so we are allowed to start this procedure
            return true;
        }
        else
        {
            // Feature bits for this procedure not present in featureSetLink
            return false;
        }
    }
}

Bool Ble_LlcpProcedureSchedulingAllowed(gpBleLlcp_ProcedureId_t procedureId, Bool masterRole)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_INVALID_PROCEDURE_ROLE_COMBINATIONS; i++)
    {
        if( Ble_FrameworkGlobalContext.invalidProcedureActions[i].procedureId == procedureId &&
            Ble_FrameworkGlobalContext.invalidProcedureActions[i].masterRole == masterRole
        )
        {
            return false;
        }
    }

    return true;
}

/*
 * Returns the first idle procedure in the queue that should be executed.
 * We will only return the procedure if it is allowed to start at this moment.
 * This function will NOT update the queue head.
 */
Ble_LlcpProcedureContext_t* Ble_LlcpGetFirstAllowedProcedure(Ble_LlcpLinkContext_t* pContext)
{
    if(Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries != 0)
    {
        UInt8 rdPointer = Ble_LlcpProcedureQueue[pContext->connId].queueRdPointer;
        UIntLoop queueIndex = 0;
        Ble_LlcpProcedureContext_t* pProcedure;

        pProcedure = Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[rdPointer];

        while(pProcedure != NULL && queueIndex < BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION)
        {
            if(!gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_RUNNING_IDX))
            {
                if(Ble_LlcpIsProcedureStartAllowed(pProcedure, queueIndex))
                {
                    return pProcedure;
                }
            }
            queueIndex++;
            rdPointer = BLE_LLCP_QUEUE_PTR_INCREMENT(rdPointer);
            pProcedure = Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[rdPointer];
        }
    }

    return NULL;
}

/*
 * Identical to Ble_LlcpGetFirstAllowedProcedure, but update the queue head after getting the procedure.
 */
Ble_LlcpProcedureContext_t* Ble_LlcpSelectNextLocalProcedure(Ble_LlcpLinkContext_t* pContext)
{
    Ble_LlcpProcedureContext_t* pProcedure;

    pProcedure = Ble_LlcpGetFirstAllowedProcedure(pContext);

    if(pProcedure != NULL)
    {
        Ble_LlcpProcedureQueue[pContext->connId].queueRdPointer = BLE_LLCP_QUEUE_PTR_INCREMENT(Ble_LlcpProcedureQueue[pContext->connId].queueRdPointer);
    }

    return pProcedure;
}

void Ble_LlcpTriggerProcedureQueue(void* pArgs)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;

    pContext = (Ble_LlcpLinkContext_t*) pArgs;

    if(pContext == NULL)
    {
        // pContext should always be a valid pointer
        GP_LOG_PRINTF("Trigger procedure queue: invalid context",0);
        GP_ASSERT_DEV_INT(false);
        return;
    }

    if (pContext->terminationOngoing)
    {
        GP_LOG_PRINTF("Trigger procedure queue: Termination Ongoing",0);
        return;
    }

    if(!BLE_IS_INT_CONN_HANDLE_VALID(pContext->connId))
    {
        // connection identifier should always be valid
        GP_LOG_PRINTF("Trigger procedure queue: invalid connId: %x",0, pContext->connId);
        GP_ASSERT_DEV_INT(false);
        return;
    }

    pProcedure = Ble_LlcpGetFirstAllowedProcedure(pContext);

    if(pProcedure == NULL)
    {
        // Stop if there is no procedure that can be executed now
        GP_LOG_PRINTF("No procedure to execute, stop!",0);
        return;
    }

    if(Ble_LlcpIsProcedureAlwaysAllowed(pProcedure->procedureId))
    {
        Ble_LlcpProcedureContext_t* pNextQueuedProcedure = NULL;

        // Make sure we allow the procedure to run (could have been disabled by a remote-initiated encryption procedure)
        gpBleLlcpFramework_EnableProcedureHandling(pContext, true, true);

        // Always allowed procedure ==> cleanup local procedure when present
        if(pContext->pProcedureLocal != NULL)
        {
            Bool removePrstUser;

            // Only remove PRST when it is still running for this procedure
            removePrstUser = (pContext->pProcedureLocal->expectedRxPdu != gpBleLlcp_OpcodeInvalid);

            pContext->pProcedureLocal->result = gpHci_ResultDifferentTransactionCollision;
            gpBleLlcpFramework_ProcedureStateSet(pContext->pProcedureLocal, BLE_LLCP_PROCEDURE_INTERRUPTED_BY_TERMINATION_IDX);

            Ble_LlcpCleanupProcedure(pContext->pProcedureLocal, pContext, removePrstUser);
        }

        // Cleanup any procedure that is queued before the termination procedure
        pNextQueuedProcedure = Ble_LlcpSelectNextLocalProcedure(pContext);

        while(pNextQueuedProcedure != pProcedure)
        {
            pNextQueuedProcedure->result = gpHci_ResultDifferentTransactionCollision;
            Ble_LlcpCleanupProcedure(pNextQueuedProcedure, pContext, false);
            pNextQueuedProcedure = Ble_LlcpSelectNextLocalProcedure(pContext);
        }

        pContext->pProcedureLocal = pNextQueuedProcedure;

    }

    if(!pContext->localProcedureFlowEnabled)
    {
        // Local procedure flow was disabled. Wait until re-enabled
        GP_LOG_PRINTF("Local procedure flow disabled for connection %x",0, pContext->connId);
        return;
    }

    if(pContext->pProcedureLocal == NULL)
    {
        pContext->pProcedureLocal = Ble_LlcpSelectNextLocalProcedure(pContext);
    }

    if(pContext->pProcedureLocal == NULL)
    {
        GP_ASSERT_DEV_INT(false);
        return;
    }

    if(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_PURGED_IDX))
    {
        GP_LOG_PRINTF("Immediate cleanup of purged procedure: %u",0, pProcedure->procedureId);
        // PRST was not started, no need to stop in cleanup
        Ble_LlcpCleanupProcedure(pProcedure, pContext, false);
        return;
    }

    // We need this check here before starting a procedure, as it could be possible that the features were not known
    // when this procedure was queued. This can happen when a connection is setup: the feature exchange procedure might
    // not be finished or it is possible that the remote does not support the slave feature request procedure.
    if(!Ble_LlcpIsProcedureSupportedOnLink(pContext, pProcedure->procedureId))
    {
        GP_LOG_PRINTF("Unsupported procedure %u on connection %u",0, pProcedure->procedureId, pContext->connId);
        pProcedure->result = gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature;
        // PRST was not started, no need to stop in cleanup
        Ble_LlcpCleanupProcedure(pProcedure, pContext, false);
        return;
    }

    Ble_LlcpStartProcedureLocal(pContext, pProcedure);
}

void Ble_LlcpStartProcedureLocal(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    GP_LOG_PRINTF("Start Local procedure: %x",0,pProcedure->procedureId);

    Ble_LlcpFrameworkAction_t action;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    GP_ASSERT_DEV_INT(BLE_IS_LLCP_PROCEDURE_VALID(pProcedure->procedureId));

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    Ble_LlcpAddPrstUser(pContext);

    // Check if there is a user callback on procedure start - and call it
    Ble_LlcpTriggerUserCbIfRegistered(pContext, pProcedure, true);

    action = Ble_LlcpTriggerStartIfRegistered(pContext, pProcedure);

    if(action == Ble_LlcpFrameworkActionContinue)
    {
        GP_ASSERT_DEV_INT(pContext != NULL);

        Ble_LlcpSendPduIfResourceAvailable(pContext, pProcedure, Ble_LlcpGetStartPduFromProcedureDescriptor(pDescriptor), 0, NULL);
        gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_RUNNING_IDX);
    }
    else
    {
        Ble_LlcpStartProcedureCommon(pContext, pProcedure, action);
    }
}

void Ble_LlcpStartProcedureRemote(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    Ble_LlcpFrameworkAction_t action;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    GP_LOG_PRINTF("Start Remote procedure: %x",0,pProcedure->procedureId);

    // Make sure there is a descriptor for the remote procedure
    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    Ble_LlcpAddPrstUser(pContext);

    if(pDescriptor->procedureDataLength > 0)
    {
        // In case the descriptor specified a data lengt, allocate room to store procedure specific data
        gpBleLlcpFramework_AddProcedureData(pProcedure, pDescriptor->procedureDataLength, NULL);
    }

    Ble_LlcpTriggerUserCbIfRegistered(pContext, pProcedure, true);

    action = Ble_LlcpTriggerStartIfRegistered(pContext, pProcedure);

    if(action == Ble_LlcpFrameworkActionContinue)
    {
        GP_ASSERT_DEV_INT(pContext != NULL);

        action = Ble_LlcpTriggerStoreCtrDataIfRegistered(pContext, pContext->pProcedureRemote, pPdLoh, opcode);

        if(action == Ble_LlcpFrameworkActionReject)
        {
            gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX);
            Ble_LlcpSendReject(pContext, pProcedure, opcode, pProcedure->result);
            return;
        }
        else if(action == Ble_LlcpFrameworkActionRejectWithUnknownRsp)
        {
            gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX);
            Ble_LlcpSendUnknownRsp(pContext, pProcedure, opcode);
            return;
        }

        action = Ble_LlcpTriggerPduReceivedIfRegistered(pContext, pProcedure, opcode);

        if(action == Ble_LlcpFrameworkActionContinue)
        {
            Ble_LlcpSendPduIfResourceAvailable(pContext, pProcedure, pProcedure->currentTxPdu, 0, NULL);
            gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_RUNNING_IDX);
        }
        else
        {
            Ble_LlcpStartProcedureCommon(pContext, pProcedure, action);
        }
    }
    else
    {
        if(action == Ble_LlcpFrameworkActionWaitForEmptyTxQueue)
        {
            // For remote procedures, it is currently not possible to return Ble_LlcpFrameworkActionWaitForEmptyTxQueue status from the startCb.
            // The reason for this is that we cannot determine the next PDU to send (and the store is not called for the Rx PDU).
            GP_ASSERT_DEV_INT(false);
        }

        Ble_LlcpStartProcedureCommon(pContext, pProcedure, action);
    }
}

void Ble_LlcpStartProcedureCommon(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpFrameworkAction_t action)
{
    if(action == Ble_LlcpFrameworkActionWaitForEmptyTxQueue)
    {
        GP_LOG_PRINTF("Register empty queue cb for proc: %x",0, pProcedure->procedureId);

        if(!Ble_LlcpIsEmptyQueueCallbackPending(pContext))
        {
            // Register empty queue callback (only when no other procedure is waiting for it)
            gpBle_DataTxQueueRegisterEmptyQueueCallback(pContext->connId, Ble_LlcpEmptyQueueCb);
        }

        gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX);
    }
    else if(action == Ble_LlcpFrameworkActionPause)
    {
        // This branch seems to be triggered only from the remote procedure start
        // Consider moving it to that specific function
        if(pProcedure->currentTxPdu == gpBleLlcp_OpcodeInvalid)
        {
            // The procedure was started, the PRST is running, but we do not need to send a PDU ==> Remove PRST
            Ble_LlcpRemovePrstUser(pContext);
        }
        // In this case, we need to wait on an external trigger before resuming procedure execution
        GP_LOG_PRINTF("Wait for external trigger before continuing procedureId %d ",2, pProcedure->procedureId);

        gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_RUNNING_IDX);
    }
    else if(action == Ble_LlcpFrameworkActionStop)
    {
        // The procedure context decides we do not need to continue
        Ble_LlcpCleanupProcedure(pProcedure, pContext, true);
    }
    else
    {
        // The procedure returned an action that is not known by the framework
        GP_ASSERT_DEV_INT(false);
    }
}

void Ble_LlcpFreeProcedure(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    // pProcedure can be NULL, in case the termination procedure requested to stop a connection
    if(pProcedure != NULL)
    {
        if(pContext->pProcedureRemote == pProcedure)
        {
            pContext->pProcedureRemote = NULL;
            gpPoolMem_Free(Ble_LlcpProcedureQueue[pContext->connId].pRemoteProcedure);
            Ble_LlcpProcedureQueue[pContext->connId].pRemoteProcedure = NULL;
        }
        else
        {
            UIntLoop i;

            if(pContext->pProcedureLocal == pProcedure)
            {
                pContext->pProcedureLocal = NULL;
            }

            for(i = 0; i < BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION; i++)
            {
                if(Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i] == pProcedure)
                {
                    gpPoolMem_Free(Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i]);
                    Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[i] = NULL;
                }
            }
        }
    }
}

void Ble_LlcpFreeProcedureData(Ble_LlcpProcedureContext_t* pProcedure)
{
    GP_ASSERT_DEV_INT(pProcedure != NULL);

    if(pProcedure->dataLength > 0)
    {
        GP_ASSERT_DEV_INT(pProcedure->pData != NULL);

        gpPoolMem_Free(pProcedure->pData);
        pProcedure->dataLength = 0;
    }
}

void Ble_LlcpFrameworkFreeProcedureAndData(Ble_LlcpProcedureContext_t* pProcedure)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(pProcedure->connId);
    GP_ASSERT_DEV_INT(pContext != NULL);

    Ble_LlcpFreeProcedureData(pProcedure);
    Ble_LlcpFreeProcedure(pContext, pProcedure);
}

void Ble_LlcpFrameworkFreeProcedureAndDataDelayed(void* pArgs)
{
    Ble_LlcpProcedureContext_t* pProcedure;

    pProcedure = (Ble_LlcpProcedureContext_t*) pArgs;
    GP_ASSERT_DEV_INT(pProcedure != NULL);

    Ble_LlcpFrameworkFreeProcedureAndData(pProcedure);
}

void Ble_LlcpCleanupProcedure(Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpLinkContext_t* pContext, Bool removePrstUser)
{
    Bool notifyHost;
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(pContext->connId));

    GP_LOG_PRINTF("Cleanup procId %u",0,pProcedure->procedureId);

    if(removePrstUser)
    {
        Ble_LlcpRemovePrstUser(pContext);
    }

    if(pProcedure == pContext->pProcedureLocal)
    {
        GP_ASSERT_DEV_INT(Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries > 0);

        Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries--;

        if (0 == Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries)
        {
            gpBleLlcp_ProhibitSlaveLatency(pContext->connId, false, Ble_ProhibitSlaveLatency_LocalProcedure);
        }

#ifndef GP_COMP_UNIT_TEST
        if( (false == gpHci_commandsEnabled())
            &&
        // number of available storage resources is back above a certain level. TBD
            (Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries < (BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION - 2))
        )
        {
            gpHci_resumeCommands();
        }
#endif//ndef GP_COMP_UNIT_TEST
    }
    else
    {
        gpBleLlcp_ProhibitSlaveLatency(pContext->connId, false, Ble_ProhibitSlaveLatency_RemoteProcedure);
    }

    if(!gpSched_ExistsEventArg(Ble_LlcpTriggerProcedureQueue, pContext) && Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries > 0)
    {
        // Trigger queue if entries are pending
        gpSched_ScheduleEventArg(0, Ble_LlcpTriggerProcedureQueue, pContext);
    }

    if(!GP_HCI_RESULT_VALID(pProcedure->result))
    {
        // In case the result has not been changed explicitely, assume success
        pProcedure->result = gpHci_ResultSuccess;
    }

    if(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX))
    {
        // The other procedure can also have the wait for empty queue flag.
        // However, entering the cleanup with the wait for empty queue flag set, is always an indication of a connection that is terminated.
        // So although 2 procedures can wait for the empty queue, this will not be a problem, as the connection will be terminated anyhow.

        gpBle_DataTxQueueUnregisterEmptyQueueCallback(pContext->connId);
        gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX);
    }

    if(pProcedure->result == gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature)
    {
        // The procedure is not supported by the remote. Clear the feature bit(s) in the featureSet
        const Ble_LlcpProcedureDescriptor_t* pDescriptor;
        UInt64 featureMask;

        pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);
        featureMask = pDescriptor->featureMask;
        if(pProcedure->procedureId == gpBleLlcp_ProcedureIdFeatureExchange && !pContext->masterConnection)
        {
            featureMask = BM(gpBleConfig_FeatureIdSlaveFeatureExchange);
        }
        pContext->allowedProcedures &= ~(featureMask);
    }

    notifyHost = !(gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_PURGED_IDX) || gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_INTERRUPTED_BY_TERMINATION_IDX));

    // notifyHost is a hint from the framework to determine whether the procedure should send an event
    Ble_LlcpTriggerFinishedIfRegistered(pContext, pProcedure, notifyHost);

    // Check if there is a user callback on procedure completion - and call it
    Ble_LlcpTriggerUserCbIfRegistered(pContext, pProcedure, false);

    if(pContext->terminationOngoing && (pContext->pProcedureLocal == pProcedure) && (pProcedure->procedureId == gpBleLlcp_ProcedureIdTermination))
    {
        // Termination ongoing. This means the procedure context can still be accessed after return from this function.
        // Therefor, we should schedule the freeing of the procedure
        gpSched_ScheduleEventArg(0, Ble_LlcpFrameworkFreeProcedureAndDataDelayed, (void*)pProcedure);
    }
    else
    {
        // No termination, we can follow normal cleanup flow
        Ble_LlcpFrameworkFreeProcedureAndData(pProcedure);
    }
}

void Ble_LlcpAddPrstUser(Ble_LlcpLinkContext_t* pContext)
{
    // Add a user to the PRST and make sure it does not overflow
    GP_LOG_PRINTF("Add PRST %x -> %x",0, pContext->nrOfProceduresUsingPrst, pContext->nrOfProceduresUsingPrst+1);

    pContext->nrOfProceduresUsingPrst++;
    GP_ASSERT_DEV_INT(pContext->nrOfProceduresUsingPrst <= BLE_LLCP_MAX_NR_OF_SIMULTANEOUS_ACTIVE_PROCEDURES);
}

void Ble_LlcpEnablePrst(Ble_LlcpLinkContext_t* pContext, Bool enable)
{
    GP_LOG_PRINTF("Enable PRST %x",0, enable);

    gpSched_UnscheduleEventArg(Ble_LlcpPrstExpired, pContext);

    if(enable)
    {
        gpSched_ScheduleEventArg(MS_TO_US(BLE_LLCP_PROCEDURE_RESPONSE_TIMEOUT_MS), Ble_LlcpPrstExpired, pContext);
    }
}

void Ble_LlcpRemovePrstUser(Ble_LlcpLinkContext_t* pContext)
{
    // Remove a PRST user and stop when no users left
    GP_ASSERT_DEV_INT(pContext->nrOfProceduresUsingPrst > 0);

    GP_LOG_PRINTF("remove PRST %x -> %x",0,pContext->nrOfProceduresUsingPrst,pContext->nrOfProceduresUsingPrst-1);

    pContext->nrOfProceduresUsingPrst--;

    if(pContext->nrOfProceduresUsingPrst == 0)
    {
        // Stop the procedure response timer in case no procedure needs it anymore
        gpSched_UnscheduleEventArg(Ble_LlcpPrstExpired, pContext);
    }
}

void Ble_LlcpPrstExpired(void* pArg)
{
    Ble_LlcpLinkContext_t* pContext = (Ble_LlcpLinkContext_t*)pArg;

    GP_LOG_SYSTEM_PRINTF("PRST expired",0);

    if(pContext != NULL)
    {
        UIntLoop i;
        Bool procedureMatched = false;
        Ble_LlcpProcedureContext_t* pProcedures[] = {pContext->pProcedureLocal, pContext->pProcedureRemote};

        GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(pContext->connId));

        for(i = 0; i < number_of_elements(pProcedures); i++)
        {
            if(pProcedures[i] != NULL)
            {
                procedureMatched = true;
                pProcedures[i]->result = gpHci_ResultLMPResponseTimeoutLLResponseTimeout;
                Ble_LlcpCleanupProcedure(pProcedures[i], pContext, true);
            }
        }

        // Check there was a procedure pending
        GP_ASSERT_DEV_INT(procedureMatched);

        gpBle_StopConnection(pContext->hciHandle, gpHci_ResultLMPResponseTimeoutLLResponseTimeout);
    }
    else
    {
        // No context found
        GP_ASSERT_DEV_INT(false);
    }
}

void Ble_LlcpSendPduIfResourceAvailable(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t opcode, UInt8 dataLength, UInt8* pData)
{
    Ble_LlcpPduResource_t* pResource = NULL;
    UIntLoop i = 0;

    if (!BLE_LLCP_OPCODE_VALID(opcode))
    {
        GP_LOG_PRINTF("Trying to send invalid LLCP PDU: %x",0,opcode);
        GP_ASSERT_DEV_INT(false);
        return;
    }

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(BLE_LLCP_OPCODE_VALID(opcode));

    while(i < BLE_LLCP_MAX_NR_OF_BUFFERED_RESOURCES_PER_LINK)
    {
        if(pContext->pdus[i].length == 0)
        {
            pResource = &pContext->pdus[i];
            break;
        }
        i++;
    }

    if(pResource == NULL)
    {
        // Both local and remote are taken, can only be the case when we initiate the termination procedure
        pResource = &pContext->pdus[0];
    }

    GP_ASSERT_DEV_INT(pResource != NULL);

    if(dataLength == 0)
    {
        UInt8 ctrDataLength = 0;

        // No data specified, ask procedure if we need any data in this pdu

        // We should always have a get ctr data function registered
        Ble_LlcpTriggerGetCtrDataIfRegistered(pContext, pProcedure, &opcode, &ctrDataLength, &pResource->data[BLE_CONTROL_PDU_CTR_DATA_OFFSET]);

        // Fill in the opcode
        pResource->data[BLE_CONTROL_PDU_OPCODE_OFFSET] = opcode;
        pResource->length++;

        pResource->length += ctrDataLength;
    }
    else
    {
        // Fill in the opcode
        pResource->data[BLE_CONTROL_PDU_OPCODE_OFFSET] = opcode;
        pResource->length++;

        // ctr data is specified, add it
        MEMCPY(&pResource->data[pResource->length], pData, dataLength);
        //gpPd_PrependWithUpdate(&pdLoh, dataLength, pData);
        pResource->length += dataLength;
    }

    if(pProcedure == NULL)
    {
        pResource->origin = Ble_LlcpPduResourceOriginUnmatched;
    }
    else if(pProcedure == pContext->pProcedureLocal)
    {
        pResource->origin = Ble_LlcpPduResourceOriginLocal;
    }
    else if(pProcedure == pContext->pProcedureRemote)
    {
        pResource->origin = Ble_LlcpPduResourceOriginRemote;
    }
    else
    {
        // Should not happen
        GP_ASSERT_DEV_INT(false);
    }

    Ble_LlcpTrySendingPdu(pContext, pProcedure, pResource);
}

void Ble_LlcpTrySendingPdu(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpPduResource_t* pResource)
{
    gpHci_Result_t result;
    gpPd_Loh_t pdLoh;
    Bool prstUserRemoved = false;

    GP_ASSERT_DEV_INT(pContext);

    // In case we are local, check if local procedure execution is allowed at this moment
    if(Ble_LlcpIsLocalInitiatedProcedure(pContext, pProcedure))
    {
        if(!pContext->localProcedureFlowEnabled)
        {
            GP_LOG_PRINTF("Local procedures suspended - pdu tx not allowed",0);
            return;
        }
    }
    else
    {
        if(!pContext->remoteProcedureFlowEnabled)
        {
            GP_LOG_PRINTF("Remote procedures suspended - pdu tx not allowed",0);
            return;
        }
    }

    // Get a resource to transmit the PDU
    Ble_LlcpGetTxResource(&pdLoh, pContext);

    if(gpPd_CheckPdValid(pdLoh.handle) == gpPd_ResultValidHandle)
    {
        Ble_LlcpFrameworkAction_t afterQueuedAction;

        gpPd_PrependWithUpdate(&pdLoh, pResource->length, pResource->data);

        if(pProcedure != NULL)
        {
            pProcedure->pdHandle = pdLoh.handle;
            pProcedure->currentTxPdu = gpPd_ReadByte(pdLoh.handle, pdLoh.offset);
            GP_LOG_PRINTF("Queue PDU",0);
            BLE_PRINT_LLCP_OPCODE(pProcedure->currentTxPdu);
        }
        else
        {
            GP_LOG_PRINTF("Queue PDU proc null",0);
            BLE_PRINT_LLCP_OPCODE(gpPd_ReadByte(pdLoh.handle, pdLoh.offset));
        }

#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
        // Do not send packets with CTE to devices that do not support receiving CTE : cfr Vol 6 Part B $2.4
        if ( pProcedure != NULL &&
             pProcedure->currentTxPdu == gpBleLlcp_OpcodeCteRsp &&
             gpBleLlcp_IsFeatureSupported(pContext->hciHandle, gpBleConfig_FeatureIdReceivingConstantToneExtension) == gpBleLlcp_FeatureStatus_Supported)
        {
            gpBleData_CteOptions_t cteOptions;

            Ble_CteProcedureData_t* pData;

            pData = (Ble_CteProcedureData_t*)pProcedure->pData;

            // Cleanup
            cteOptions.includeCte = true;
            cteOptions.cteDurationUnit = pData->minCteLengthUnit;// CTE length limits are enforced by LLCP protocol : max(min(pData->minCteLengthUnit,20),2);
            cteOptions.cteType = pData->cteType;
            result = gpBle_DataTxQueueRequest(pContext->connId, pdLoh, Ble_LLID_Control, &cteOptions);
        }
        else
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

        {
            result = gpBle_DataTxQueueRequest(pContext->connId, pdLoh, Ble_LLID_Control, NULL);
        }

        // Should always be success, because we checked this in advance
        GP_ASSERT_DEV_INT(result == gpHci_ResultSuccess);

        // Free resource by setting its length to zero
        pResource->length = 0;

        if(pProcedure == NULL)
        {
            // Nothing more to be done when there is no procedure context
            return;
        }

        // Each LLCP Control PDU that is queued for transmission reset the PRST (only when it was a response to a valid PDU)
        Ble_LlcpEnablePrst(pContext, true);

        if(Ble_LlcpCommonIsRejectPdu(pProcedure->currentTxPdu))
        {
            GP_LOG_PRINTF("Auto cleanup after sending reject (ext)/unknown",0);

            pProcedure->expectedRxPdu = gpBleLlcp_OpcodeInvalid;

            // Queueing a reject/unknown_rsp terminates the procedure
            afterQueuedAction = Ble_LlcpFrameworkActionStop;
        }
        else
        {
            afterQueuedAction = Ble_LlcpTriggerPduQueuedIfRegistered(pContext, pProcedure, pProcedure->currentTxPdu);
        }

        if(pProcedure->expectedRxPdu == gpBleLlcp_OpcodeInvalid)
        {
            // No expected RX PDU after queueing. PRST can be stopped for this procedure
            Ble_LlcpRemovePrstUser(pContext);
            prstUserRemoved = true;
        }

        if(afterQueuedAction == Ble_LlcpFrameworkActionStop)
        {
            // Procedure can be terminated immediately after queueing the PDU
            Ble_LlcpCleanupProcedure(pProcedure, pContext, !prstUserRemoved);
        }
    }
    else
    {
        // PDU should stay in queue!
    }
}

void Ble_LlcpSendUnsolicitedPdu(Ble_LlcpLinkContext_t* pContext, UInt8* pPduOpcodeAndPayload, UInt8 PduLength, gpBleData_CteOptions_t *pcteOptions)
{
    gpPd_Loh_t pdLoh;

    if(!pContext->localProcedureFlowEnabled)
    {
        GP_LOG_PRINTF("Local procedures suspended - unsolicited pdu tx not allowed",0);
        return;
    }

    // Get a resource to transmit the PDU
    Ble_LlcpGetTxResource(&pdLoh, pContext);

    if(gpPd_CheckPdValid(pdLoh.handle) == gpPd_ResultValidHandle)
    {
        Ble_LlcpPduResource_t tempResource;
        Ble_LlcpPduResource_t *pResource;
        gpHci_Result_t result;

        tempResource.length = PduLength;
        MEMCPY(tempResource.data,pPduOpcodeAndPayload,PduLength);
        pResource = &tempResource;

        gpPd_PrependWithUpdate(&pdLoh, pResource->length, pResource->data);

        GP_LOG_PRINTF("Queue PDU proc null",0);
        BLE_PRINT_LLCP_OPCODE(gpPd_ReadByte(pdLoh.handle, pdLoh.offset));

#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
        // Do not send packets with CTE to devices that do not support receiving CTE : cfr Vol 6 Part B $2.4
        if ( pcteOptions != NULL &&
             gpBleLlcp_IsFeatureSupported(pContext->hciHandle, gpBleConfig_FeatureIdReceivingConstantToneExtension) == gpBleLlcp_FeatureStatus_Supported )
        {
            result = gpBle_DataTxQueueRequest(pContext->connId, pdLoh, Ble_LLID_Control, pcteOptions);
        }
        else
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */
        {
            result = gpBle_DataTxQueueRequest(pContext->connId, pdLoh, Ble_LLID_Control, NULL);
        }
        NOT_USED(result);
        GP_LOG_PRINTF("Sending unsolicited LLCP PDU: result = %d",0,result);
    }
    else
    {
        GP_LOG_PRINTF("No Tx resource for sending unsolicited LLCP PDU",0);
    }
}

void Ble_LlcpGetTxResource(gpPd_Loh_t* pPdLoh, Ble_LlcpLinkContext_t* pContext)
{
    pPdLoh->handle = gpBle_DataTxQueueAllocatePd(pContext->connId, Ble_DataChannelTxQueueCallerLlcp);

    if(gpPd_CheckPdValid(pPdLoh->handle) != gpPd_ResultValidHandle)
    {
        // No pd available now
        return;
    }

    pPdLoh->offset = GPBLEDATACOMMON_PDU_FOOTER_MAX_OFFSET;
    pPdLoh->length = 0;
}

void Ble_LlcpSendReject(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t opcode, gpHci_Result_t reason)
{
    gpBleLlcp_Opcode_t rejectOpcode;
    UInt8 rejectPayload[2];
    UInt8* pRejectPayload = &rejectPayload[0];
    UInt8 rejectPayloadLength = 0;

    if(Ble_LlcpIsExtendedRejectSupported(pContext, pProcedure))
    {
        // extended reject possible, first byte was original opcode
        pRejectPayload[rejectPayloadLength++] = opcode;
        rejectOpcode = gpBleLlcp_OpcodeRejectExtInd;
    }
    else
    {
        // no extended reject
        rejectOpcode = gpBleLlcp_OpcodeRejectInd;
    }

    // Reason is last payload byte
    pRejectPayload[rejectPayloadLength++] = reason;
    GP_LOG_PRINTF("Extended reject used: %x",0,(rejectOpcode == gpBleLlcp_OpcodeRejectExtInd));

    Ble_LlcpSendPduIfResourceAvailable(pContext, pProcedure, rejectOpcode, rejectPayloadLength, pRejectPayload);
}

void Ble_LlcpSendUnknownRsp(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t opcode)
{
    Ble_LlcpSendPduIfResourceAvailable(pContext, pProcedure, gpBleLlcp_OpcodeUnknownRsp, 1, &opcode);
}

Bool Ble_LlcpPduTransmitted(Ble_LlcpProcedureContext_t* pProcedure, gpPd_Handle_t pdHandle)
{
    Ble_LlcpLinkContext_t* pContext;

    if(pProcedure == NULL)
    {
        return false;
    }

    pContext = Ble_GetLinkContext(pProcedure->connId);
    GP_ASSERT_DEV_INT(pContext != NULL);

    if(pdHandle == pProcedure->pdHandle)
    {
        Ble_LlcpFrameworkAction_t afterTxAction;
        Bool removePrstUser = true;

        GP_LOG_PRINTF("TX",0);
        BLE_PRINT_LLCP_OPCODE(pProcedure->currentTxPdu);

        afterTxAction = Ble_LlcpTriggerPduTxedIfRegistered(pContext, pProcedure, pProcedure->currentTxPdu);

        // Make sure to invalidate the stored pdHandle (to make sure, we won't trigger it again)
        pProcedure->pdHandle = GP_PD_INVALID_HANDLE;

        if(pProcedure->expectedRxPdu == gpBleLlcp_OpcodeInvalid)
        {
            // Expected RX PDU should not change between queued and transmitted callback.
            // This means PRST was already removed, so no need to do it here
            removePrstUser = false;
        }

        if(afterTxAction == Ble_LlcpFrameworkActionPause)
        {
            gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_RUNNING_IDX);
            GP_LOG_PRINTF("Tx done, pause procedure",0);
            return true;
        }
        else if(afterTxAction == Ble_LlcpFrameworkActionStop)
        {
            // Finish if we are told to
            Ble_LlcpCleanupProcedure(pProcedure, pContext, removePrstUser);
            return true;
        }
        else if(pProcedure->expectedRxPdu == gpBleLlcp_OpcodeInvalid && afterTxAction == Ble_LlcpFrameworkActionContinue)
        {
            // There is no reason to continue procedure when no respone is expected.
            GP_ASSERT_DEV_INT(pContext != NULL);
            return true;
        }
        return true;
    }
    return false;
}

void Ble_LlcpEmptyQueueCb(Ble_IntConnId_t connId)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext);

    GP_LOG_PRINTF("Ble empty queue for conn %i",0,connId);

    // Not interested anymore when the queue is empty
    gpBle_DataTxQueueUnregisterEmptyQueueCallback(pContext->connId);

    pProcedure = pContext->pProcedureLocal;

    if(pProcedure && gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX))
    {
        const Ble_LlcpProcedureDescriptor_t* pDescriptor;

        pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

        GP_ASSERT_DEV_INT(pDescriptor != NULL);

        GP_LOG_PRINTF("local procedure %i waiting on empty queue",0, pProcedure->procedureId);

        gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX);
        // Local procedure was waiting on empty queue, resume
        gpBleLlcpFramework_ResumeProcedure(pContext, pProcedure, Ble_LlcpGetStartPduFromProcedureDescriptor(pDescriptor));
    }

    pProcedure = pContext->pProcedureRemote;

    if(pProcedure && gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX))
    {
        GP_LOG_PRINTF("remote procedure %i waiting on empty queue",0, pProcedure->procedureId);

        gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX);
        // Remote procedure was waiting on empty queue, resume
        gpBleLlcpFramework_ResumeProcedure(pContext, pProcedure,  pProcedure->currentTxPdu);
    }
}

Bool Ble_LlcpIsEmptyQueueCallbackPending(Ble_LlcpLinkContext_t* pContext)
{
    Ble_LlcpProcedureContext_t* pProcedure;

    GP_ASSERT_DEV_INT(pContext);

    pProcedure = pContext->pProcedureLocal;

    if(pProcedure && gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX))
    {
        return true;
    }

    pProcedure = pContext->pProcedureRemote;

    if(pProcedure && gpBleLlcpFramework_ProcedureStateGet(pProcedure, BLE_LLCP_PROCEDURE_WAITING_ON_EMPTY_QUEUE_IDX))
    {
        return true;
    }

    return false;
}

Bool Ble_LlcpProcessIncomingPdu(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode, gpPd_Loh_t pdLoh)
{
    Bool freePd = true;

    // First do a length check. We never accept PDUs that have an incorrect length
    if(!Ble_LlcpIsCtrDataLengthValid(opcode, pdLoh.length))
    {
        // The length is incorrect. Respond with LL_UNKNOWN_RSP
        Ble_LlcpSendUnknownRsp(pContext, NULL, opcode);
        return freePd;
    }

    if(pContext->terminationOngoing && opcode != gpBleLlcp_OpcodeTerminateInd)
    {
        GP_LOG_PRINTF("Termination ongoing => drop non-termination PDU",0);
        return freePd;
    }

    if(Ble_LlcpFollowupLocal(pContext, pContext->rxLlcpOpcode, &pdLoh, &freePd))
    {
        return freePd;
    }
    if(Ble_LlcpFollowupRemote(pContext, pContext->rxLlcpOpcode, &pdLoh, &freePd))
    {
        return freePd;
    }
    if(Ble_LlcpFollowupNewRemote(pContext, pContext->rxLlcpOpcode, &pdLoh, &freePd))
    {
        return freePd;
    }

    return freePd;
}

gpBleLlcp_ProcedureId_t Ble_LlcpGetProcedureIdFromStartPdu(gpBleLlcp_Opcode_t rxLlcpOpcode)
{
    UIntLoop i;
    gpBleLlcp_ProcedureId_t procedureId = gpBleLlcp_ProcedureIdInvalid;


    for(i = 0; i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES; i++)
    {
        const Ble_LlcpProcedureDescriptor_t* pDescriptor = Ble_FrameworkGlobalContext.procedures[i].pDescriptor;

        if(Ble_LlcpProcedureHasValidPduDescriptors(pDescriptor))
        {
            // The first PDU in the list is considered the starting PDU
            if(pDescriptor->pPduDescriptors[0].opcode == rxLlcpOpcode)
            {
                return Ble_FrameworkGlobalContext.procedures[i].procedureId;
            }
        }
    }

    return procedureId;
}

Bool Ble_LlcpFollowupCommon(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh, Bool* pFreePd)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;
    gpBleLlcp_Opcode_t txOpcode = gpBleLlcp_OpcodeInvalid;
    Bool matched = false;
    Bool prstUserRemoved = false;

    // Reject invalid PDUs (only when encryption is not running)
    if((pContext->masterConnection && pContext->remoteProcedureFlowEnabled) ||
       (!pContext->masterConnection && pContext->localProcedureFlowEnabled) )
    {
        if(!Ble_LlcpIsValidPdu(pContext, rxOpcode))
        {
            // Stop here, handling done in Ble_LlcpIsValidPdu
            return true;
        }
    }

    if(rxOpcode == gpBleLlcp_OpcodeTerminateInd)
    {
        if(!pProcedure->localInit && pProcedure->procedureId == gpBleLlcp_ProcedureIdTermination)
        {
            // A second remote-initiated termination procedure should be ignored
            GP_LOG_PRINTF("Received a second remote termination procedure, drop!",0);
            return true;
        }

        Bool removePrstUser;

        // If we receive a termination while there is a procedure running, we first cleanup the current procedure
        GP_LOG_PRINTF("termination while proc registered: %x",0,pProcedure->procedureId);

        // Only remove PRST when it is still running for this procedure
        removePrstUser = (pProcedure->expectedRxPdu != gpBleLlcp_OpcodeInvalid);

        pContext->terminationOngoing = true;

        // Indicate that the current procedure was interrupted by the termination procedure
        pProcedure->result = gpHci_ResultDifferentTransactionCollision;
        gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_INTERRUPTED_BY_TERMINATION_IDX);

        Ble_LlcpCleanupProcedure(pProcedure, pContext, removePrstUser);

        // return false, allows termination procedure to follow the flow in Ble_LlcpFollowupNewRemote
        return false;
    }
    else if(pProcedure->expectedRxPdu == gpBleLlcp_OpcodeInvalid)
    {
        // No PDU expected, skip followup
        return false;
    }
    else if(pProcedure->expectedRxPdu == rxOpcode)
    {
        matched = true;
        action = Ble_LlcpTriggerStoreCtrDataIfRegistered(pContext, pProcedure, pPdLoh, rxOpcode);

        if(action == Ble_LlcpFrameworkActionReject)
        {
            gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX);
            Ble_LlcpSendReject(pContext, pProcedure, rxOpcode, pProcedure->result);
            return matched;
        }
        else if(action == Ble_LlcpFrameworkActionRejectWithUnknownRsp)
        {
            gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_LOCALLY_REJECTED_IDX);
            Ble_LlcpSendUnknownRsp(pContext, pProcedure, rxOpcode);
            return matched;
        }
        else if(action == Ble_LlcpFrameworkActionPause)
        {
            return matched;
        }

        action = Ble_LlcpTriggerPduReceivedIfRegistered(pContext, pProcedure, rxOpcode);
        txOpcode = pProcedure->currentTxPdu;
    }
    else
    {
        action = Ble_LlcpTriggerUnexpectedPduReceivedIfRegistered(pContext, pProcedure, rxOpcode, pPdLoh);

        if(pContext->masterConnection && !pContext->remoteProcedureFlowEnabled)
        {
            if(action == Ble_LlcpFrameworkActionContinue)
            {
                // At this point, the procedure needs to be suspended.
                // Make sure to update the pd (because we have cut off one byte)
                pPdLoh->length++;
                pPdLoh->offset--;
                MEMCPY(&pContext->queuedPdLoh, pPdLoh, sizeof(gpPd_Loh_t));
                *pFreePd = false;
                matched = true;
                return matched;
            }
        }
    }

    if((txOpcode == gpBleLlcp_OpcodeInvalid || action == Ble_LlcpFrameworkActionStop) && matched)
    {
        // No further tx action required. PRST can be stopped for this procedure
        Ble_LlcpRemovePrstUser(pContext);
        prstUserRemoved = true;
    }

    switch(action)
    {
        case Ble_LlcpFrameworkActionContinue:
        {
            if(txOpcode != gpBleLlcp_OpcodeInvalid)
            {
                // Only send next PDU if txOpcode was changed (can only be the case when an expected PDU is received)
                Ble_LlcpSendPduIfResourceAvailable(pContext, pProcedure, txOpcode, 0, NULL);
            }
            break;
        }
        case Ble_LlcpFrameworkActionPause:
        {
            matched = true;
            gpBleLlcpFramework_ProcedureStateClear(pProcedure, BLE_LLCP_PROCEDURE_RUNNING_IDX);
            break;
        }
        case Ble_LlcpFrameworkActionStop:
        {
            matched = true;
            Ble_LlcpCleanupProcedure(pProcedure, pContext, !prstUserRemoved);
            break;
        }
        default:
        {
            GP_ASSERT_DEV_INT(false);       // Should not happen
            break;
        }
    }

    return matched;
}

Bool Ble_LlcpFollowupLocal(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh, Bool* pFreePd)
{
    Ble_LlcpProcedureContext_t* pProcedure;

    GP_ASSERT_DEV_INT(pContext != NULL);

    pProcedure = pContext->pProcedureLocal;

    if(pProcedure == NULL)
    {
        // No local procedure registered
        return false;
    }

    if(!pContext->localProcedureFlowEnabled && pContext->pProcedureRemote != NULL)
    {
        // Local procedure flow not allowed. This can only be the case when we are the slave.
        // In this case, the master (remote) is running the encryption procedure.
        // So this PDU will first be handed to the remote procedure (Ble_LlcpFollowupRemote) in case it is not an UNKNOWN_RSP PDU
        // If it is an UNKNOWN_RSP PDU, it will be matched to the local procedure first, if it doesn't match, the link should still disconnect
        if(rxOpcode != gpBleLlcp_OpcodeUnknownRsp)
        {
            return false;
        }
    }

    return Ble_LlcpFollowupCommon(pContext, pProcedure, rxOpcode, pPdLoh, pFreePd);
}

Bool Ble_LlcpFollowupRemote(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh, Bool* pFreePd)
{
    Ble_LlcpProcedureContext_t* pProcedure;

    GP_ASSERT_DEV_INT(pContext != NULL);

    pProcedure = pContext->pProcedureRemote;

    if(pProcedure == NULL)
    {
        // No remote procedure registered
        return false;
    }

    return Ble_LlcpFollowupCommon(pContext, pProcedure, rxOpcode, pPdLoh, pFreePd);
}

Bool Ble_LlcpFollowupNewRemote(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh, Bool* pFreePd)
{
    gpBleLlcp_ProcedureId_t remoteProcedure;
    Ble_LlcpProcedureContext_t* pProcedure;

    GP_ASSERT_DEV_INT(pContext != NULL);

    GP_LOG_PRINTF("Ble_LlcpFollowupNewRemote: rx %x",0,rxOpcode);

    // Check here first if this PDU was acceptable
    if(!Ble_LlcpIsValidPdu(pContext, rxOpcode))
    {
        return false;
    }

    if(Ble_LlcpCommonIsRejectPdu(rxOpcode))
    {
        GP_LOG_PRINTF("Unmatched reject/unknown ==> drop",0);
        // Reject could not be matched with an existing procedure ==> drop
        return false;
    }

    if(pContext->pProcedureRemote != NULL)
    {
        // Second (non-termination) procedure initiated by remote ==> reject
        Ble_LlcpSendReject(pContext, NULL, pContext->rxLlcpOpcode, gpHci_ResultLMPPDUNotAllowed);
        return true;
    }

    remoteProcedure = Ble_LlcpGetProcedureIdFromStartPdu(rxOpcode);

    pContext->pProcedureRemote = Ble_LlcpAllocateProcedure(pContext, false, true, remoteProcedure);

    pProcedure = pContext->pProcedureRemote;

    if(pProcedure != NULL)
    {
        // This is used by some procedures to know the reject opcode ==> fill in
        pProcedure->expectedRxPdu = rxOpcode;

        Ble_LlcpStartProcedureRemote(pContext, pProcedure, pPdLoh, rxOpcode);
    }
    else
    {
        // It was not possible to associate a procedure with this PDU.
        // This means it is a known PDU, but not appropriate at this time ==> send reject.
        GP_LOG_PRINTF("Could not find procedure belonging to PDU %x", 0, rxOpcode);
        Ble_LlcpSendReject(pContext, NULL, pContext->rxLlcpOpcode, gpHci_ResultLMPPDUNotAllowed);
    }

    return true;
}

Bool Ble_LlcpPduReceived(Ble_LlcpLinkContext_t* pContext, gpPd_Loh_t pdLoh)
{
    Bool freePd = true;

    if(pContext == NULL || !Ble_LlcpProcedureQueue[pContext->connId].active)
    {
        // When there is no valid context or the procedure queue is not active anymore (termination ongoing)
        GP_LOG_PRINTF("No valid context found ==> drop PDU %x",0, gpPd_ReadByte(pdLoh.handle, pdLoh.offset));
        // Ignore packets on non-connected or termination-pending links
        return freePd;
    }

    if(pdLoh.length == 0)
    {
        GP_LOG_PRINTF("Drop empty Control PDU",0);
        return freePd;
    }

    gpPd_ReadWithUpdate(&pdLoh, 1, &pContext->rxLlcpOpcode);

    GP_LOG_PRINTF("PDU RX ",0);
    BLE_PRINT_LLCP_OPCODE(pContext->rxLlcpOpcode);

    freePd = Ble_LlcpProcessIncomingPdu(pContext, pContext->rxLlcpOpcode, pdLoh);

    return freePd;
}

void Ble_LlcpProcessQueuedPdu(void* pArg)
{
    Ble_LlcpLinkContext_t* pContext = (Ble_LlcpLinkContext_t*)pArg;

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_LOG_PRINTF("LlcpProcessQueuedPdu line %d ",0, __LINE__);
    if(gpPd_CheckPdValid(pContext->queuedPdLoh.handle) == gpPd_ResultValidHandle)
    {
        // PD will be freed in this function
        Bool freePd = Ble_LlcpPduReceived(pContext, pContext->queuedPdLoh);

        // Pd should always be freed immediately if it was a queued one
        GP_ASSERT_DEV_INT(freePd);

        Ble_RMFreeResource(pContext->connId, pContext->queuedPdLoh.handle);
        pContext->queuedPdLoh.handle = GP_PD_INVALID_HANDLE;
    }
}

gpHci_Result_t Ble_LlcpGetCollisionResult(gpBleLlcp_ProcedureId_t localProcedureId, gpBleLlcp_ProcedureId_t remoteProcedureId)
{
    GP_ASSERT_DEV_INT(BLE_IS_LLCP_PROCEDURE_VALID(localProcedureId));
    GP_ASSERT_DEV_INT(BLE_IS_LLCP_PROCEDURE_VALID(remoteProcedureId));

    if(remoteProcedureId == gpBleLlcp_ProcedureIdConnectionParamRequest && localProcedureId == gpBleLlcp_ProcedureIdConnectionUpdate)
    {
        // This is also the same procedure
        return gpHci_ResultLLProcedureCollision;
    }

    if(localProcedureId == remoteProcedureId)
    {
        return gpHci_ResultLLProcedureCollision;
    }
    else
    {
        return gpHci_ResultDifferentTransactionCollision;
    }
}

gpHci_Result_t Ble_LlcpProcedureNoCollisionDetected(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode)
{
    Ble_LlcpProcedureContext_t* pProcedure;

    if(!pContext->masterConnection)
    {
        // No collision rules when local device is slave
        return gpHci_ResultSuccess;
    }

    pProcedure = pContext->pProcedureLocal;

    if(pProcedure == NULL)
    {
        // There can't be any collision if we do not have a local procedure running as master
        return gpHci_ResultSuccess;
    }

    if(Ble_LlcpIsInstantProcedure(pProcedure->procedureId))
    {
        UIntLoop i;

        for(i = 0; i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES; i++)
        {
            gpBleLlcp_ProcedureId_t incomingProcedureId = Ble_LlcpGetProcedureIdFromStartPdu(opcode);

            if(BLE_IS_LLCP_PROCEDURE_VALID(incomingProcedureId) && Ble_LlcpIsInstantProcedure(incomingProcedureId))
            {
                return Ble_LlcpGetCollisionResult(pProcedure->procedureId, incomingProcedureId);
            }
        }
    }

    return gpHci_ResultSuccess;
}

// Check if the local role (master/slave) is allowed to TX/RX a PDU with the given opcode
gpHci_Result_t Ble_LlcpProcedurePduAllowed(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode, UInt8 allowedRoleMask)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES; i++)
    {
        UIntLoop j;
        const Ble_LlcpProcedureDescriptor_t* pDescriptor;

        pDescriptor = Ble_FrameworkGlobalContext.procedures[i].pDescriptor;

        if(Ble_LlcpProcedureHasValidPduDescriptors(pDescriptor))
        {
            for(j = 0; j < pDescriptor->nrOfPduDescriptors; j++)
            {
                if(opcode == pDescriptor->pPduDescriptors[j].opcode)
                {
                    if((allowedRoleMask & pDescriptor->pPduDescriptors[j].pduFlags) == 0)
                    {
                        // No match, which means it was not allowed to send/receive this PDU
                        return gpHci_ResultLMPPDUNotAllowed;
                    }

                    break;
                }
            }
        }
    }

    return gpHci_ResultSuccess;
}

gpHci_Result_t Ble_LlcpAcceptIncomingPdu(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode)
{
    gpHci_Result_t result;

    // Incoming PDU, role mask should have the remote role as input
    result = Ble_LlcpProcedurePduAllowed(pContext, opcode, Ble_LlcpRoleToRoleMask(!pContext->masterConnection));

    if(result == gpHci_ResultSuccess)
    {
        // Check if there is a collision with local procedures
        result = Ble_LlcpProcedureNoCollisionDetected(pContext, opcode);
    }

    return result;
}

Bool Ble_LlcpIsValidPdu(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_Opcode_t opcode)
{
    gpHci_Result_t rejectReason;

    // Check if the remote was allowed to send this PDU
    rejectReason = Ble_LlcpAcceptIncomingPdu(pContext, pContext->rxLlcpOpcode);

    if(!BLE_LLCP_OPCODE_VALID(pContext->rxLlcpOpcode))
    {
        // Unknown opcode
        GP_LOG_PRINTF("Invalid opcode %x",0, pContext->rxLlcpOpcode);
        Ble_LlcpSendPduIfResourceAvailable(pContext, NULL, gpBleLlcp_OpcodeUnknownRsp, 1, &pContext->rxLlcpOpcode);
        return false;
    }
    else if(rejectReason != gpHci_ResultSuccess)
    {
        // Opcode rejected (collision or invalid PDU received from remote)
        GP_LOG_PRINTF("Reject opcode %x (reason: %x)",0, pContext->rxLlcpOpcode, rejectReason);
        Ble_LlcpSendReject(pContext, NULL, pContext->rxLlcpOpcode, rejectReason);
        return false;
    }

    return true;
}

Bool Ble_LlcpIsCtrDataLengthValid(gpBleLlcp_Opcode_t opcode, UInt8 ctrDataLength)
{
    // First check all registered procedures
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES; i++)
    {
        const Ble_LlcpProcedureDescriptor_t* pDescriptor = Ble_FrameworkGlobalContext.procedures[i].pDescriptor;

        if(Ble_LlcpProcedureHasValidPduDescriptors(pDescriptor))
        {
            UIntLoop j;

            const gpBleLlcpFramework_PduDescriptor_t* pPduDescriptor = pDescriptor->pPduDescriptors;

            for(j = 0; j < pDescriptor->nrOfPduDescriptors; j++)
            {
                if(pPduDescriptor[j].opcode == opcode && pPduDescriptor[j].ctrDataLength != ctrDataLength)
                {
                    GP_LOG_PRINTF("Opcode 0x%x: wrong length (exp: %u, rec: %u)",0, opcode, pPduDescriptor[j].ctrDataLength, ctrDataLength);
                    return false;
                }
            }
        }
    }

    // We don't know this PDU, which means we cannot determine wheter or not the ctrDataLength is okay
    return true;
}

Ble_LlcpFrameworkAction_t Ble_LlcpTriggerQueueingNeededIfRegistered(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_ProcedureId_t procedureId)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(pDescriptor->cbQueueingNeeded)
    {
        action = pDescriptor->cbQueueingNeeded(pContext);
    }

    return action;
}

Ble_LlcpFrameworkAction_t Ble_LlcpTriggerStartIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    Ble_LlcpFrameworkAction_t action = Ble_LlcpFrameworkActionContinue;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(pDescriptor->cbProcedureStart)
    {

        if(pDescriptor->procedureDataLength > 0)
        {
            GP_ASSERT_DEV_INT(pProcedure->pData != NULL);
        }

        action = pDescriptor->cbProcedureStart(pContext, pProcedure);
    }

    return action;
}

void Ble_LlcpTriggerGetCtrDataIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t* pOpcode, UInt8* pCtrDataLength, UInt8* pCtrData)
{
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(pDescriptor->cbGetCtrData)
    {
        pDescriptor->cbGetCtrData(pContext, pProcedure, pOpcode, pCtrDataLength, pCtrData);
    }

    // Check if local role is allowed to send this PDU. Check is done after cbGetCtrData as the opcode can be changed
    if(Ble_LlcpProcedurePduAllowed(pContext, *pOpcode, Ble_LlcpRoleToRoleMask(pContext->masterConnection)) != gpHci_ResultSuccess)
    {
        GP_LOG_PRINTF("Local role (m: %x) not allowed to send PDU %x",0, pContext->masterConnection, *pOpcode);
        // The local role is not allowed to send this PDU
        GP_ASSERT_DEV_INT(false);
    }
}

Ble_LlcpFrameworkAction_t Ble_LlcpTriggerStoreCtrDataIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpPd_Loh_t* pPdLoh, gpBleLlcp_Opcode_t opcode)
{
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;
    Ble_LlcpFrameworkAction_t afterStoreAction = Ble_LlcpFrameworkActionContinue;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(pDescriptor->cbStoreCtrData)
    {
        if(pDescriptor->procedureDataLength > 0)
        {
            GP_ASSERT_DEV_INT(pProcedure->pData != NULL);
        }

        afterStoreAction = pDescriptor->cbStoreCtrData(pContext, pProcedure, pPdLoh, opcode);
    }

    return afterStoreAction;
}

Ble_LlcpFrameworkAction_t Ble_LlcpTriggerPduQueuedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t afterQueuedAction = Ble_LlcpFrameworkActionContinue;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(Ble_LlcpProcedurePduAllowed(pContext, txOpcode, Ble_LlcpRoleToRoleMask(pContext->masterConnection)) != gpHci_ResultSuccess)
    {
        // The local role is not allowed to queue/send this PDU
        GP_ASSERT_DEV_INT(false);
    }

    if(pDescriptor->cbPduQueued != NULL)
    {
        afterQueuedAction = pDescriptor->cbPduQueued(pContext, pProcedure, txOpcode);
    }

    return afterQueuedAction;
}

Ble_LlcpFrameworkAction_t Ble_LlcpTriggerPduTxedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t txOpcode)
{
    Ble_LlcpFrameworkAction_t afterTxAction = Ble_LlcpFrameworkActionContinue;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(Ble_LlcpProcedurePduAllowed(pContext, txOpcode, Ble_LlcpRoleToRoleMask(pContext->masterConnection)) != gpHci_ResultSuccess)
    {
        // The local role was not allowed to send this PDU
        GP_ASSERT_DEV_INT(false);
    }

    if(pDescriptor->cbPduTransmitted)
    {
        afterTxAction = pDescriptor->cbPduTransmitted(pContext, pProcedure, txOpcode);
    }

    return afterTxAction;
}

// The procedure should update the next PDU to send (pProcedure->currentTxPdu)
Ble_LlcpFrameworkAction_t Ble_LlcpTriggerPduReceivedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode)
{
    Ble_LlcpFrameworkAction_t rxPduFollowup = Ble_LlcpFrameworkActionContinue;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(pDescriptor->cbPduReceived)
    {
        rxPduFollowup = pDescriptor->cbPduReceived(pContext, pProcedure, rxOpcode);
    }

    return rxPduFollowup;
}

Ble_LlcpFrameworkAction_t Ble_LlcpTriggerUnexpectedPduReceivedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t rxOpcode, gpPd_Loh_t* pPdLoh)
{
    Ble_LlcpFrameworkAction_t rxPduFollowup = Ble_LlcpFrameworkActionContinue;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(pDescriptor->cbUnexpectedPduReceived)
    {
        rxPduFollowup = pDescriptor->cbUnexpectedPduReceived(pContext, pProcedure, rxOpcode, pPdLoh);
    }

    return rxPduFollowup;
}

void Ble_LlcpTriggerFinishedIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool notifyHost)
{
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pProcedure->procedureId);

    GP_ASSERT_DEV_INT(pContext != NULL);
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(pDescriptor->cbFinished != NULL)
    {
        pDescriptor->cbFinished(pContext, pProcedure, notifyHost);
    }
}

void Ble_LlcpTriggerUserCbIfRegistered(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, Bool start)
{
    gpBleLlcpFramework_cbProcedureCb_t userCb = NULL;
    UIntLoop i;

    for(i = 0; i < GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS; i++)
    {
        if(Ble_FrameworkGlobalContext.callbacks[i].procedureId == pProcedure->procedureId)
        {
            if(start)
            {
                userCb = Ble_FrameworkGlobalContext.callbacks[i].pCallbacks->cbProcedureStart;
            }
            else
            {
                userCb = Ble_FrameworkGlobalContext.callbacks[i].pCallbacks->cbProcedureDone;
            }
            break;
        }
    }

    if(userCb != NULL)
    {
        userCb(pProcedure);
    }
}

const Ble_LlcpProcedureDescriptor_t* BleLlcpFramework_GetProcedureDescriptor(gpBleLlcp_ProcedureId_t procedureId)
{
    UIntLoop i;

    if(!BLE_IS_LLCP_PROCEDURE_VALID(procedureId))
    {
        return NULL;
    }

    for(i = 0; i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES; i++)
    {
        if(Ble_FrameworkGlobalContext.procedures[i].procedureId == procedureId)
        {
            return Ble_FrameworkGlobalContext.procedures[i].pDescriptor;
        }
    }

    return NULL;
}

Bool Ble_LlcpIsLocalInitiatedProcedure(Ble_LlcpLinkContext_t* pContext,Ble_LlcpProcedureContext_t* pProcedure)
{
    if(pContext->pProcedureLocal != NULL && pContext->pProcedureLocal == pProcedure)
    {
        return true;
    }

    return false;
}

Bool Ble_LlcpIsInstantProcedure(gpBleLlcp_ProcedureId_t procedureId)
{
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;

    if(!BLE_IS_LLCP_PROCEDURE_VALID(procedureId))
    {
        return false;
    }

    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(procedureId);

    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    return GPBLELLCPFRAMEWORK_PROCEDURE_FLAGS_INSTANT_GET(pDescriptor->procedureFlags);
}

Bool Ble_LlcpCommonIsRejectPdu(gpBleLlcp_Opcode_t opcode)
{
    if(opcode == gpBleLlcp_OpcodeUnknownRsp || opcode == gpBleLlcp_OpcodeRejectInd || opcode == gpBleLlcp_OpcodeRejectExtInd)
    {
        return true;
    }

    return false;
}

Bool Ble_LlcpIsExtendedRejectSupported(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure)
{
    if(pContext == NULL)
    {
        // No extended reject possible if context not available
        return false;
    }
    else if(gpBleLlcp_IsFeatureSupported(pContext->hciHandle, gpBleConfig_FeatureIdExtendedRejectIndication) == gpBleLlcp_FeatureStatus_Supported)
    {
        // If indicated in features, extended reject can be used
        return true;
    }
    else if(pProcedure == NULL)
    {
        // No extended reject possible when procedure is not available
        return false;
    }
    else if( pProcedure->procedureId == gpBleLlcp_ProcedureIdConnectionParamRequest ||
             pProcedure->procedureId == gpBleLlcp_ProcedureIdPhyUpdate)
    {
        // These procedures require extended reject indication
        return true;
    }

    return false;
}

UInt8 Ble_LlcpRoleToRoleMask(Bool masterTransmitter)
{
    if(masterTransmitter)
    {
        return GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_CENTRAL;
    }
    else
    {
        return GPBLELLCPFRAMEWORK_PDU_ROLE_MASK_PERIPHERAL;
    }
}

gpBleLlcp_Opcode_t Ble_LlcpGetStartPduFromProcedureDescriptor(const Ble_LlcpProcedureDescriptor_t* pDescriptor)
{
    if(!Ble_LlcpProcedureHasValidPduDescriptors(pDescriptor))
    {
        // Return invalid in case parameters are not correct
        return gpBleLlcp_OpcodeInvalid;
    }

    // The startPdu is always the first PDU in the pdu descriptor list
    return pDescriptor->pPduDescriptors[0].opcode;
}

Bool Ble_LlcpProcedureHasValidPduDescriptors(const Ble_LlcpProcedureDescriptor_t* pDescriptor)
{
    if(pDescriptor == NULL || pDescriptor->nrOfPduDescriptors == 0 || pDescriptor->pPduDescriptors == NULL)
    {
        // This points to an error in the definition of the descriptor
        GP_ASSERT_DEV_INT(false);
        return false;
    }

    return true;
}

#ifdef GP_LOCAL_LOG
void PrintLLCP_Opcode(gpBleLlcp_Opcode_t opcode)
{
    if(opcode >= number_of_elements(Ble_LlcpOpcodeStrings))
    {
        GP_LOG_SYSTEM_PRINTF(" Unknown",0);
    }
    else
    {
#if defined(GP_COMP_UNIT_TEST)
        GP_LOG_SYSTEM_PRINTF( " %s " ,0,Ble_LlcpOpcodeStrings[opcode]);
#else
        GP_LOG_SYSTEM_PRINTF(Ble_LlcpOpcodeStrings[opcode],0);
#endif
    }
}
#endif // GP_DIVERSITY_DEVELOPMENT

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleLlcpFramework_Init(void)
{
    UIntLoop i;
    UIntLoop j;

    // Initialize the global context
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_INVALID_PROCEDURE_ROLE_COMBINATIONS; i++)
    {
        Ble_FrameworkGlobalContext.invalidProcedureActions[i].procedureId = gpBleLlcp_ProcedureIdInvalid;
        Ble_FrameworkGlobalContext.invalidProcedureActions[i].masterRole = false;
    }

    for(i = 0; i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES; i++)
    {
        Ble_FrameworkGlobalContext.procedures[i].procedureId = gpBleLlcp_ProcedureIdInvalid;
    }

    for(i = 0; i < GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS; i++)
    {
        Ble_FrameworkGlobalContext.callbacks[i].procedureId = gpBleLlcp_ProcedureIdInvalid;
        Ble_FrameworkGlobalContext.callbacks[i].pCallbacks = NULL;
    }

    // Make sure that all queued procedure pointers point to nothing
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        for(j = 0; j < BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION; j++)
        {
            Ble_LlcpProcedureQueue[i].queuedProcedures[j] = NULL;
        }
    }

    // Features are 64 bits, but procedure context (Ble_LlcpProcedureDescriptor_t) only has storage for 32 bits (RAM optimisation)
    // We need to extend when growing beyond 32 features
    // This has impact as we need to use UInt64 in that case (requires 8 byte alignment of Ble_LlcpProcedureDescriptor_t)
    GP_COMPILE_TIME_VERIFY(gpBleConfig_FeatureIdInvalid < 64);
}

void gpBleLlcpFramework_Reset(Bool firstReset)
{
    UIntLoop i;

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        BleLlcpFramework_CloseConnectionInternal(i, true);

        Ble_LlcpProcedureQueue[i].active = false;
    }
}

void gpBleLlcpFramework_OpenConnection(Ble_IntConnId_t connId)
{
    GP_LOG_PRINTF("LlcpFramework open conn %u",0, connId);

    Ble_LlcpProcedureQueue[connId].active = true;
}

void gpBleLlcpFramework_CloseConnection(Ble_IntConnId_t connId)
{
    BleLlcpFramework_CloseConnectionInternal(connId, false);
}

void gpBleLlcpFramework_RegisterProcedure(gpBleLlcp_ProcedureId_t procedureId, const Ble_LlcpProcedureDescriptor_t* pDescriptor)
{
    UIntLoop i;

    GP_ASSERT_DEV_INT(pDescriptor != NULL);
    GP_ASSERT_DEV_INT(BLE_IS_LLCP_PROCEDURE_VALID(procedureId));

    GP_LOG_PRINTF("Register procedure %x",0, procedureId);

    if(!Ble_LlcpProcedureHasValidPduDescriptors(pDescriptor))
    {
        GP_LOG_SYSTEM_PRINTF("No valid PDUS specified for procedure %u",0, procedureId);
        GP_ASSERT_DEV_INT(false);
    }

    for(i = 0; i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES; i++)
    {
        if(Ble_FrameworkGlobalContext.procedures[i].procedureId == gpBleLlcp_ProcedureIdInvalid)
        {
            Ble_FrameworkGlobalContext.procedures[i].procedureId = procedureId;
            Ble_FrameworkGlobalContext.procedures[i].pDescriptor = pDescriptor;
            break;
        }
    }

    // Make sure there was room for this procedure
    GP_ASSERT_DEV_INT(i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES);
}

void gpBleLlcpFramework_RegisterProcedureCallbacks(gpBleLlcp_ProcedureId_t procedureId, const gpBleLlcpFramework_ProcedureUserCallbacks_t* pCallbacks)
{
    UIntLoop i;

    GP_ASSERT_DEV_INT(BLE_IS_LLCP_PROCEDURE_VALID(procedureId));

    for(i = 0; i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES; i++)
    {
        if(Ble_FrameworkGlobalContext.callbacks[i].procedureId == gpBleLlcp_ProcedureIdInvalid)
        {
            Ble_FrameworkGlobalContext.callbacks[i].procedureId = procedureId;
            Ble_FrameworkGlobalContext.callbacks[i].pCallbacks = pCallbacks;
            break;
        }
    }

    // Make sure there was room for this procedure
    GP_ASSERT_DEV_INT(i < BLE_LLCP_NUMBER_OF_SUPPORTED_PROCEDURES);
}

void gpBleLlcpFramework_RegisterInvalidProcedureAction(gpBleLlcp_ProcedureId_t procedureId, Bool masterRole)
{
    UIntLoop i;

    // Initialize the global context
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_INVALID_PROCEDURE_ROLE_COMBINATIONS; i++)
    {
        if(Ble_FrameworkGlobalContext.invalidProcedureActions[i].procedureId == gpBleLlcp_ProcedureIdInvalid)
        {
            Ble_FrameworkGlobalContext.invalidProcedureActions[i].procedureId = procedureId;
            Ble_FrameworkGlobalContext.invalidProcedureActions[i].masterRole = masterRole;
            break;
        }
    }

    GP_ASSERT_DEV_INT(i < BLE_LLCP_MAX_NR_OF_INVALID_PROCEDURE_ROLE_COMBINATIONS);
}

gpHci_Result_t gpBleLlcpFramework_StartProcedure(Ble_IntConnId_t connId, gpBleLlcpFramework_StartProcedureDescriptor_t* pStart)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;
    const Ble_LlcpProcedureDescriptor_t* pDescriptor;
    Ble_LlcpFrameworkAction_t queueingNeededAction;

    pContext = Ble_GetLinkContext(connId);
    pDescriptor = BleLlcpFramework_GetProcedureDescriptor(pStart->procedureId);

    GP_ASSERT_DEV_INT(pContext);
    GP_ASSERT_DEV_INT(BLE_IS_LLCP_PROCEDURE_VALID(pStart->procedureId));
    GP_ASSERT_DEV_INT(pDescriptor != NULL);

    if(!Ble_LlcpIsProcedureSupportedOnLink(pContext, pStart->procedureId))
    {
        // The remote indicated that it does not support this procedure
        GP_LOG_PRINTF("Procedure %u not supported by remote",0, pStart->procedureId);
        return gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature;
    }

    // Check if the local role is allowed to start this procedure
    if(!Ble_LlcpProcedureSchedulingAllowed(pStart->procedureId, pContext->masterConnection))
    {
        GP_LOG_PRINTF("Procedure %x not allowed by role %x",0,pStart->procedureId, !pContext->masterConnection);
        return gpHci_ResultCommandDisallowed;
    }

    queueingNeededAction = Ble_LlcpTriggerQueueingNeededIfRegistered(pContext, pStart->procedureId);

    if(queueingNeededAction == Ble_LlcpFrameworkActionStop)
    {
        // The procedure decided that queueing is not needed, because the requested info might already be available.
        return gpHci_ResultSuccess;
    }

    pProcedure = Ble_LlcpAllocateProcedure(pContext, true, pStart->controllerInit, pStart->procedureId);

    if(pProcedure == NULL)
    {
        GP_LOG_PRINTF("No room to store procedure %x on link %x",0,pStart->procedureId, connId);
        return gpHci_ResultMemoryCapacityExceeded;
    }

    if(pDescriptor->procedureDataLength > 0)
    {
        gpBleLlcpFramework_AddProcedureData(pProcedure, pDescriptor->procedureDataLength, (UInt8*)&pStart->procedureData);
    }

    return gpHci_ResultSuccess;
}

void gpBleLlcpFramework_ResumeProcedure(Ble_LlcpLinkContext_t* pContext, Ble_LlcpProcedureContext_t* pProcedure, gpBleLlcp_Opcode_t pdu)
{
    GP_ASSERT_DEV_INT(pContext != NULL);

    GP_LOG_PRINTF("Resume procedure with pdu: %x",0,pdu);

    Ble_LlcpSendPduIfResourceAvailable(pContext, pProcedure, pdu, 0, NULL);

    gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_RUNNING_IDX);
}

Ble_LlcpProcedureContext_t* gpBleLlcpFramework_GetProcedure(Ble_LlcpLinkContext_t* pContext, Bool local)
{
    GP_ASSERT_DEV_INT(pContext != NULL);

    if(local)
    {
        return pContext->pProcedureLocal;
    }
    else
    {
        return pContext->pProcedureRemote;
    }
}

void gpBleLlcpFramework_StopActiveProcedure(gpHci_ConnectionHandle_t connHandle, Bool local)
{
    Ble_LlcpLinkContext_t* pContext;
    Ble_LlcpProcedureContext_t* pProcedure;

    pContext = Ble_GetLinkContext(gpBleLlcp_HciHandleToIntHandle(connHandle));

    GP_ASSERT_DEV_INT(pContext != NULL);

    pProcedure = gpBleLlcpFramework_GetProcedure(pContext, local);

    GP_ASSERT_DEV_INT(pProcedure != NULL);

    // PRST was already removed
    Ble_LlcpCleanupProcedure(pProcedure, pContext, false);
}

Ble_LlcpProcedureContext_t* gpBleLlcpFramework_PurgeFirstQueuedProcedure(Ble_LlcpLinkContext_t* pContext, gpBleLlcp_ProcedureId_t procedureId)
{
    if(Ble_LlcpProcedureQueue[pContext->connId].nrOfQueuedEntries != 0)
    {
        UInt8 rdPointer = Ble_LlcpProcedureQueue[pContext->connId].queueRdPointer;
        UIntLoop attempts = 0;
        while(attempts < BLE_LLCP_MAX_NR_OF_QUEUED_PROCEDURES_PER_CONNECTION)
        {
            Ble_LlcpProcedureContext_t* pProcedure = Ble_LlcpProcedureQueue[pContext->connId].queuedProcedures[rdPointer];

            // Only consider purging if procedure does not match the current local one
            if(pProcedure != NULL && pContext->pProcedureLocal != pProcedure)
            {
                if(pProcedure->procedureId == procedureId)
                {
                    gpBleLlcpFramework_ProcedureStateSet(pProcedure, BLE_LLCP_PROCEDURE_PURGED_IDX);
                    return pProcedure;
                }
            }
            attempts++;
            rdPointer = BLE_LLCP_QUEUE_PTR_INCREMENT(rdPointer);
        }
    }

    // No matching procedure found
    return NULL;
}

void gpBleLlcpFramework_EnableProcedureHandling(Ble_LlcpLinkContext_t* pContext, Bool local, Bool enable)
{
    GP_ASSERT_DEV_INT(pContext != NULL);
    Bool wasEnabledBefore;

    if(local)
    {
        wasEnabledBefore = pContext->localProcedureFlowEnabled;
        pContext->localProcedureFlowEnabled = enable;
    }
    else
    {
        wasEnabledBefore = pContext->remoteProcedureFlowEnabled;
        pContext->remoteProcedureFlowEnabled = enable;
    }

    if(enable && !wasEnabledBefore)
    {
        // Artificial resource available trigger
        Ble_LlcpResourceAvailableInd(pContext->connId);

        if(!local)
        {
            gpSched_ScheduleEventArg(0, Ble_LlcpProcessQueuedPdu, pContext);
        }
    }
}

void gpBleLlcpFramework_AddProcedureData(Ble_LlcpProcedureContext_t* pProcedure, UInt8 length, UInt8* pData)
{
    GP_ASSERT_DEV_INT(pProcedure != NULL);
    GP_ASSERT_DEV_INT(pProcedure->pData == NULL);

    pProcedure->pData = GP_POOLMEM_MALLOC(length);

    if(pData == NULL)
    {
        MEMSET(pProcedure->pData, 0, length);
    }
    else
    {
        MEMCPY(pProcedure->pData, pData, length);
    }
    pProcedure->dataLength = length;
}

Bool gpBleLlcpFramework_ProcedureStateGet(Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpProcedureStateId_t state)
{
    return (pProcedure->state & (1 << state));
}

void gpBleLlcpFramework_ProcedureStateSet(Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpProcedureStateId_t state)
{
    pProcedure->state |= (1 << state);
}

void gpBleLlcpFramework_ProcedureStateClear(Ble_LlcpProcedureContext_t* pProcedure, Ble_LlcpProcedureStateId_t state)
{
    pProcedure->state &= ~(1 << state);
}



/*****************************************************************************
 *                    Public callbacks from lower layers
 *****************************************************************************/

void Ble_LlcpResourceAvailableInd(UInt8 connId)
{
    UIntLoop i;
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    GP_ASSERT_DEV_INT(pContext != NULL);

    for(i = 0; i < BLE_LLCP_MAX_NR_OF_BUFFERED_RESOURCES_PER_LINK; i++)
    {
        Ble_LlcpPduResource_t* pResource = &pContext->pdus[i];
        // There is buffered data ==> send
        if(pResource->length != 0)
        {
            Ble_LlcpProcedureContext_t* pProcedure = NULL;

            // Only pick local in case local procedures are allowed to run
            if((pResource->origin == Ble_LlcpPduResourceOriginLocal) && pContext->localProcedureFlowEnabled)
            {
                pProcedure = pContext->pProcedureLocal;
            }
            else if((pResource->origin == Ble_LlcpPduResourceOriginRemote) && pContext->remoteProcedureFlowEnabled)
            {
                pProcedure = pContext->pProcedureRemote;
            }

            Ble_LlcpTrySendingPdu(pContext, pProcedure, pResource);
        }
    }
}

// Incoming LLCP data packets
void Ble_LlcpDataInd(Ble_IntConnId_t connId, gpPd_Loh_t pdLoh)
{
    Ble_LlcpLinkContext_t* pContext;
    Bool freePd;

    pContext = Ble_GetLinkContext(connId);

    freePd = Ble_LlcpPduReceived(pContext, pdLoh);

    if(freePd)
    {
        Ble_RMFreeResource(connId, pdLoh.handle);
    }
}

void gpBleLlcpFramework_cbDataConfirm(UInt8 connId, gpPd_Handle_t pdHandle)
{
    Ble_LlcpLinkContext_t* pContext;

    pContext = Ble_GetLinkContext(connId);

    if(Ble_LlcpPduTransmitted(pContext->pProcedureLocal, pdHandle))
    {
        // Confirm can be matched to local procedure
        return;
    }

    if(Ble_LlcpPduTransmitted(pContext->pProcedureRemote, pdHandle))
    {
        // Confirm can be matched to remote procedure
        return;
    }

    // It can happen that no procedure is matched (e.g. a reject that was sent), but this is not a problem
}

