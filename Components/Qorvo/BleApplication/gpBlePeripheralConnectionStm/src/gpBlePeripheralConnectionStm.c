/*
 * Copyright (c) 2018, Qorvo Inc
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
 */

/** @file "gpBlePeripheralConnectionStm.c"
 *
 *  State machine to create and maintain connections in a controlled way.
 *
 *  Implementation of gpBlePeripheralConnectionStm
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEPERIPHERALCONNECTIONSTM

/* <CodeGenerator Placeholder> General */
/*
    Two important remaks have to be taken into account w.r.t. implementation of the gpBleConncetionStm:
    1. It is important that the flow from sending an event, obtaining and storing the state variable does not
       get interupted by other StateMachine events. To ensure this is happening, there should be no scheduled (gpSched, wsf_SendEvent)
       calls in handling the an event and executing a state action (statenameAction).
    2. The state machine is not designed to cope with reenterance, there fore ensure that NON of the gpBleConncetionStm API's
       are called from the interrupt context.
*/

#ifdef GP_DIVERSITY_DEVELOPMENT
#ifndef GP_BLE_MULTI_ADV_SUPPORTED
#define GP_LOCAL_LOG
#endif //GP_BLE_MULTI_ADV_SUPPORTED
#endif //GP_DIVERSITY_DEVELOPMENT

/* </CodeGenerator Placeholder> General */


#include "gpBlePeripheralConnectionStm.h"

/* <CodeGenerator Placeholder> Includes */
#include "gpAssert.h"
#include "gpLog.h"
#include "gpBleConnectionQueue.h"
/* </CodeGenerator Placeholder> Includes */


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */
#ifdef GP_LOCAL_LOG
#define DUMP_EVENT(Event,LinkId)  BleDumpEvent(Event,LinkId)
#define DUMP_STATE(State,LinkId)  BleDumpState(State,LinkId)
#else
#define DUMP_EVENT(Event,LinkId)
#define DUMP_STATE(State,LinkId)
#endif //GP_LOCAL_LOG

/* Macro IS_EVENT uses local variables gpBlePeripheralConnectionStm_SendEvent, DO NOT USE outside gpBlePeripheralConnectionStm_SendEvent */
#define IS_EVENT(x)   (Event == x)
/* Macro IS_CONF_SET uses local variables gpBlePeripheralConnectionStm_SendEvent, DO NOT USE outside gpBlePeripheralConnectionStm_SendEvent */
#define IS_CONF_SET(x) (stateConf & x)
/* Macro IS_CONF_CLEAR uses local variables gpBlePeripheralConnectionStm_SendEvent, DO NOT USE outside gpBlePeripheralConnectionStm_SendEvent */
#define IS_CONF_CLEAR(x) ((stateConf & x) == gpBlePeripheralConnectionStm_ConfigCleared)

/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> FunctionalMacro */
/* </CodeGenerator Placeholder> FunctionalMacro */

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> TypeDefinitions */
typedef void (*StateAction_t)(UInt8);

#define normal   0x00
#define abort    0x01
typedef UInt8 ExitType_t;

/* </CodeGenerator Placeholder> TypeDefinitions */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */

static UInt8 linkState[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS];
static UInt8 linkStateConf[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS];

/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionPrototypes */
static void BleConnectionStm_StateIdleAction(UInt8 LinkId);
static void BleConnectionStm_StateWaitConnectAction(UInt8 LinkId);
static void BleConnectionStm_StateWaitReconnectAction(UInt8 LinkId);
static void BleConnectionStm_StateBondedNoConnectionAction(UInt8 LinkId);
static void BleConnectionStm_StateConnectingAction(UInt8 LinkId);
static void BleConnectionStm_StateConnectingExitAction(UInt8 LinkId, ExitType_t ExitType);
static void BleConnectionStm_StateAbortConnectingAction(UInt8 LinkId);
static void BleConnectionStm_StateAbortConnectingExitAction(UInt8 LinkId);
static void BleConnectionStm_StateConnectedAction(UInt8 LinkId);
static void BleConnectionStm_StateConnectedSlaveSecurityRequestAction(UInt8 LinkId);
static void BleConnectionStm_StateClosingConnectionAction(UInt8 LinkId);
static void BleConnectionStm_StateBondedConnectedAction(UInt8 LinkId);
static void BleConnectionStm_StateBondedConnectedNoSecurityAction(UInt8 LinkId);
static void BleConnectionStm_StateBondedClosingConnectionAction(UInt8 LinkId);
static void BleConnectionStm_StateReconnectingAction(UInt8 LinkId);
static void BleConnectionStm_StateReconnectingExitAction(UInt8 LinkId, ExitType_t ExitType);
static void BleConnectionStm_StateReconnectingDirectedHighAction(UInt8 LinkId);
static void BleConnectionStm_StateReconnectingDirectedHighExitAction(UInt8 LinkId, ExitType_t ExitType);
static void BleConnectionStm_StateReconnectingDirectedLowAction(UInt8 LinkId);
static void BleConnectionStm_StateReconnectingDirectedLowExitAction(UInt8 LinkId, ExitType_t ExitType);
static void BleConnectionStm_StateStopReconnectingDirectExitAction(UInt8 LinkId);
static void BleConnectionStm_StateAbortReconnectingAction(UInt8 LinkId);
static void BleConnectionStm_StateAbortReconnectingExitAction(UInt8 LinkId);

#ifdef GP_LOCAL_LOG
static void BleDumpEvent(gpBlePeripheralConnectionStm_Event_t Event, UInt8 LinkId);
static void BleDumpState(gpBlePeripheralConnectionStm_State_t State, UInt8 LinkId);
#endif //GP_LOCAL_LOG

static gpBlePeripheralConnectionStm_State_t BleConnectionStm(gpBlePeripheralConnectionStm_State_t state, StateAction_t action, UInt8 LinkId);
/* </CodeGenerator Placeholder> StaticFunctionPrototypes */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */
static gpBlePeripheralConnectionStm_State_t BleConnectionStm(gpBlePeripheralConnectionStm_State_t state, StateAction_t action, UInt8 LinkId)
{
    if(action != NULL)
    {
        action(LinkId);
    }
    DUMP_STATE(state,LinkId);

    return state;
}


static void BleConnectionStm_StateIdleAction(UInt8 LinkId)
{
    /* Close connection if connection is open */
    gpBlePeripheralConnectionStm_cbCloseConnection(LinkId);
}

static void BleConnectionStm_StateWaitConnectAction(UInt8 LinkId)
{
    /* Fill adv Queue */
    gpBleConnectionQueue_Add(LinkId,ADVQUEUE_ID);
}

static void BleConnectionStm_StateWaitReconnectAction(UInt8 LinkId)
{
    /* Fill adv Queue */
    gpBleConnectionQueue_Add(LinkId,ADVQUEUE_ID);
}

static void BleConnectionStm_StateBondedNoConnectionAction(UInt8 LinkId)
{
    /* Close connection if connection is open */
    gpBlePeripheralConnectionStm_cbCloseConnection(LinkId);
}

static void BleConnectionStm_StateConnectingAction(UInt8 LinkId)
{
    /* Start advertising for current connection with parameters no connection */
    gpBlePeripheralConnectionStm_cbStartAdvertising(LinkId,  gpBlePeripheralConnectionStm_NewConnection);
}

static void BleConnectionStm_StateConnectingExitAction(UInt8 LinkId, ExitType_t ExitType)
{
    /* In normal happy flow a advertising queue should be popped here. If entries are in the queue
       a race condition can occur when the advertising is aborted. In that case a wait for adv stop confirm is needed
       before pop may occur. To cope with this condition BleConnectionStm_StateAbortConnecting is introduced. */

    if(ExitType == normal)
    {
        gpBleConnectionQueue_Pop(ADVQUEUE_ID);
    }
}

static void BleConnectionStm_StateConnectedAction(UInt8 LinkId)
{
    /* Pop Advertising Queue to allow next advertisement to start, wait for pair complete or security fail */
    /* 20181003 AdvQueuePop moved to ConnectingStateExit*/
}

static void BleConnectionStm_StateConnectedSlaveSecurityRequestAction(UInt8 LinkId)
{
    gpBlePeripheralConnectionStm_cbScheduleSecurityTimeout(LinkId);
}

static void BleConnectionStm_StateClosingConnectionAction(UInt8 LinkId)
{
    gpBlePeripheralConnectionStm_cbCloseConnection(LinkId);
}

static void BleConnectionStm_StateAbortConnectingAction(UInt8 LinkId)
{
}

static void BleConnectionStm_StateAbortConnectingExitAction(UInt8 LinkId)
{
    /* State advertising abort confirmed by HCI, pop queue to start next advertisement */
    gpBleConnectionQueue_Pop(ADVQUEUE_ID);
}

static void BleConnectionStm_StateBondedConnectedAction(UInt8 LinkId)
{
    /* No Action, storing of pairing information and keys will be handled CordioAppFrameWork.
       triggered by the callback AppSlaveSecProcDmMsg */
}

static void BleConnectionStm_StateBondedConnectedNoSecurityAction(UInt8 LinkId)
{

}

static void BleConnectionStm_StateBondedClosingConnectionAction(UInt8 LinkId)
{
    gpBlePeripheralConnectionStm_cbCloseConnection(LinkId);
}

static void BleConnectionStm_StateReconnectingAction(UInt8 LinkId)
{
    /* Start advertising, check central addresstype, if identity address is public, use direct advertising,
       if identity addres is random use non-discoverable advertising (Check, for need of address in adv data.) */
    gpBlePeripheralConnectionStm_cbStartAdvertising(LinkId, gpBlePeripheralConnectionStm_Reconnect);
}

static void BleConnectionStm_StateReconnectingExitAction(UInt8 LinkId, ExitType_t ExitType)
{
    /* In normal happy flow a advertising queue should be popped here. If entries are in the queue
       a race condition can occur when the advertising is aborted. In that case a wait for adv stop confirm is needed
       before pop may occur. To cope with this condition BleConnectionStm_StateAbortConnecting is introduced. */

    if(ExitType == normal)
    {
        gpBleConnectionQueue_Pop(ADVQUEUE_ID);
    }
}

static void BleConnectionStm_StateReconnectingDirectedHighAction(UInt8 LinkId)
{
    gpBlePeripheralConnectionStm_cbStartAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedHigh);
}

static void BleConnectionStm_StateReconnectingDirectedHighExitAction(UInt8 LinkId, ExitType_t ExitType)
{
    if(ExitType == normal)
    {
        gpBleConnectionQueue_Pop(ADVQUEUE_ID);
    }
}

static void BleConnectionStm_StateReconnectingDirectedLowAction(UInt8 LinkId)
{
    gpBlePeripheralConnectionStm_cbStartAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedLow);
}

static void BleConnectionStm_StateReconnectingDirectedLowExitAction(UInt8 LinkId, ExitType_t ExitType)
{
    if(ExitType == normal)
    {
        gpBleConnectionQueue_Pop(ADVQUEUE_ID);
    }
}

static void BleConnectionStm_StateStopReconnectingDirectExitAction(UInt8 LinkId)
{
    (void)LinkId; //unused
    gpBleConnectionQueue_Pop(ADVQUEUE_ID);
}

static void BleConnectionStm_StateAbortReconnectingAction(UInt8 LinkId)
{
}

static void BleConnectionStm_StateAbortReconnectingExitAction(UInt8 LinkId)
{
    /* State advertising abort confirmed by HCI, pop queue to start next advertisement */
    gpBleConnectionQueue_Pop(ADVQUEUE_ID);
}

#ifdef GP_LOCAL_LOG
static void BleDumpEvent(gpBlePeripheralConnectionStm_Event_t Event, UInt8 LinkId)
{
    switch (Event)
    {
        case gpBlePeripheralConnectionStm_EventConnReq:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventConnReq received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventAdvFail:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventAdvFail received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventAdvAvail:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventAdvAvail received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventConnOpen:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventConnOpen received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventConnOpenNew:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventConnOpenNew received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventConnFail:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventConnFail received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventPairComplete:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventPairComplete received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventNoPairRequest:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventNoPairRequest received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventConnCloseApp:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventConnCloseApp received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventConnCloseExt:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventConnCloseExt received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventSecurityFail:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventSecurityFail received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventSecurityEstablished:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventSecurityEstablished received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventPairFail:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventPairFail received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventBonded:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventBonded received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventUnbindApp:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventUnbindApp received",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_EventBLEHostResetComplete:
            GP_LOG_PRINTF("Link %d Event gpBlePeripheralConnectionStm_EventBLEHostResetComplete received",0,LinkId);
        break;

        default:
            GP_LOG_PRINTF("Link %d Event UNKNOWN received",0,LinkId);
        break;
    }
}

static void BleDumpState(gpBlePeripheralConnectionStm_State_t State, UInt8 LinkId)
{
    switch (State)
    {
        case gpBlePeripheralConnectionStm_StateIdle:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateIdle",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateWaitConnect:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateWaitConnect",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateConnecting:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateConnecting",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateAbortConnecting:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateAbortConnecting",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateConnected:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateConnected",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateClosingConnection:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateClosingConnection",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateBondedConnected:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateBondedConnected",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateBondedNoConnection:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateBondedNoConnection",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateBondedClosingConnection:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateBondedClosingConnection",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateWaitReconnect:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateWaitReconnect",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateReconnecting:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateReconnecting",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateReconnectingDirectedHigh:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateReconnectingDirectedHigh",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateReconnectingDirectedLow:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateReconnectingDirectedLow",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateAbortReconnecting:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateAbortReconnecting",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateBLEHostInitialising:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateBLEHostInitialising",0,LinkId);
        break;

        case gpBlePeripheralConnectionStm_StateStopReconnectingDirect:
            GP_LOG_PRINTF("Link %d StateTransition to gpBlePeripheralConnectionStm_StateStopReconnectingDirect",0,LinkId);
        break;

        default:
            GP_LOG_PRINTF("Link %d StateTransition STATE UNKNOWN",0,LinkId);

    }
}
#endif

/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_Event_t Event, UInt8 LinkId)
{
/* <CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_SendEvent */
    gpBlePeripheralConnectionStm_Result_t result = gpBlePeripheralConnectionStmResult_NotProcessed;

    if (LinkId < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS)
    {
        gpBlePeripheralConnectionStm_State_t state = linkState[LinkId];
        gpBlePeripheralConnectionStm_Config_t stateConf = linkStateConf[LinkId];

        DUMP_EVENT(Event, LinkId);

        switch (state)
        {
            case gpBlePeripheralConnectionStm_StateIdle:
                /* Here state transitions and call to newState action */
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnReq))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateWaitConnect,BleConnectionStm_StateWaitConnectAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventBonded) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigAutoRecon))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateWaitReconnect,BleConnectionStm_StateWaitReconnectAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventBonded) && IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigAutoRecon))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateWaitConnect:
                /* Here state transitions and call to newState action */
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventAdvAvail))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateConnecting,BleConnectionStm_StateConnectingAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventAdvFail))
                {
                    gpBleConnectionQueue_Remove(LinkId,ADVQUEUE_ID);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateIdle,BleConnectionStm_StateIdleAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp) || IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    gpBleConnectionQueue_Remove(LinkId,ADVQUEUE_ID);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateIdle,BleConnectionStm_StateIdleAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateConnecting:
                /* Here state transitions and call to newState action */
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpenNew) && IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateConnectingExitAction(LinkId,normal);
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateConnected,BleConnectionStm_StateConnectedAction,LinkId);

                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpenNew) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateConnectingExitAction(LinkId,normal);
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateConnected,BleConnectionStm_StateConnectedSlaveSecurityRequestAction,LinkId);

                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnFail))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateConnectingExitAction(LinkId,normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateIdle,BleConnectionStm_StateIdleAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp) || IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateConnectingExitAction(LinkId, abort);

                    gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_NewConnection,IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateAbortConnecting,BleConnectionStm_StateAbortConnectingAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateAbortConnecting:
                if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnFail))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateAbortConnectingExitAction(LinkId);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateIdle,BleConnectionStm_StateIdleAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateConnected:
                /* Here state transitions and call to newState action */
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventPairComplete))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnected,BleConnectionStm_StateBondedConnectedAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp) ||
                        ((IS_EVENT(gpBlePeripheralConnectionStm_EventSecurityFail) || IS_EVENT(gpBlePeripheralConnectionStm_EventPairFail)) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                       )
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateClosingConnection,BleConnectionStm_StateClosingConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateIdle,BleConnectionStm_StateIdleAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if((IS_EVENT(gpBlePeripheralConnectionStm_EventSecurityFail) || IS_EVENT(gpBlePeripheralConnectionStm_EventPairFail)) &&
                        (IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                       )
                {
                    /* Remain in gpBlePeripheralConnectionStm_StateConnected if security or pairing fails and Encrypted only is not set */
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateConnected,NULL,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateClosingConnection:
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateIdle,BleConnectionStm_StateIdleAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateBondedConnected:
                /* Here state transitions and call to newState action */
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedClosingConnection,BleConnectionStm_StateBondedClosingConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt) && IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigAutoRecon))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigAutoRecon))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateWaitReconnect,BleConnectionStm_StateWaitReconnectAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if(IS_EVENT(gpBlePeripheralConnectionStm_EventUnbindApp))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateClosingConnection,BleConnectionStm_StateClosingConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateBondedNoConnection:
                /* Here state transitions and call to newState action */
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnReq))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateWaitReconnect,BleConnectionStm_StateWaitReconnectAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateBondedConnectedNoSecurityAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateConnectedSlaveSecurityRequestAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if(IS_EVENT(gpBlePeripheralConnectionStm_EventUnbindApp))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateIdle,BleConnectionStm_StateIdleAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity:
                /* Here state transitions and call to newState action */
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventSecurityEstablished)||
                   (IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                  )
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnected,BleConnectionStm_StateBondedConnectedAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventSecurityFail) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp) || IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateBondedClosingConnection:
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateWaitReconnect:
                /* Here state transitions and call to newState action */
                if(IS_EVENT(gpBlePeripheralConnectionStm_EventAdvAvail))
                {
#ifdef GP_BLE_MULTI_ADV_SUPPORTED
                    if (IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigPairingOnly))
                    {
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateConnecting,BleConnectionStm_StateConnectingAction,LinkId);
                    }
                    else
#endif //GP_BLE_MULTI_ADV_SUPPORTED
                    if (IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigDirectedHighAdv))
                    {
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateReconnectingDirectedHigh,BleConnectionStm_StateReconnectingDirectedHighAction,LinkId);
                    }
                    else if (IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigDirectedLowAdv))
                    {
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateReconnectingDirectedLow,BleConnectionStm_StateReconnectingDirectedLowAction,LinkId);
                    }
                    else
                    {
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateReconnecting,BleConnectionStm_StateReconnectingAction,LinkId);
                    }
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventAdvFail))
                {
                    gpBleConnectionQueue_Remove(LinkId,ADVQUEUE_ID);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    gpBleConnectionQueue_Remove(LinkId,ADVQUEUE_ID);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateBondedConnectedNoSecurityAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    gpBleConnectionQueue_Remove(LinkId,ADVQUEUE_ID);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateConnectedSlaveSecurityRequestAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp) || IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    gpBleConnectionQueue_Remove(LinkId,ADVQUEUE_ID);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateReconnectingDirectedHigh:
                /* Here state transitions and call to newState action should occur */
                if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnFail))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingDirectedHighExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingDirectedHighExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateBondedConnectedNoSecurityAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingDirectedHighExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateConnectedSlaveSecurityRequestAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    if (IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigDirectedLowAdv))
                    {
                        /* If state has exit action, first call the exit action before calling Action of the new state */
                        BleConnectionStm_StateReconnectingDirectedHighExitAction(LinkId, abort);

                        gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedHigh, IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateReconnectingDirectedLow,BleConnectionStm_StateReconnectingDirectedLowAction,LinkId);
                    }
                    else if (IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigUndirectedAdv))
                    {
                        /* If state has exit action, first call the exit action before calling Action of the new state */
                        BleConnectionStm_StateReconnectingDirectedHighExitAction(LinkId, abort);

                        gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedHigh, IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateReconnecting,BleConnectionStm_StateReconnectingAction,LinkId);
                    }
                    else
                    {
                        /* If state has exit action, first call the exit action before calling Action of the new state */
                        BleConnectionStm_StateReconnectingDirectedHighExitAction(LinkId, normal);

                        gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedHigh, IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    }
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp))
                {
                    BleConnectionStm_StateReconnectingDirectedHighExitAction(LinkId, abort);
                    gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedHigh, IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateStopReconnectingDirect, NULL, LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateReconnectingDirectedLow:
                /* Here state transitions and call to newState action should occur */
                if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnFail))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingDirectedLowExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingDirectedLowExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateBondedConnectedNoSecurityAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingDirectedLowExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateConnectedSlaveSecurityRequestAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    if (IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigUndirectedAdv))
                    {
                        /* If state has exit action, first call the exit action before calling Action of the new state */
                        BleConnectionStm_StateReconnectingDirectedLowExitAction(LinkId, abort);

                        gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedLow, IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateReconnecting,BleConnectionStm_StateReconnectingAction,LinkId);
                    }
                    else
                    {
                        /* If state has exit action, first call the exit action before calling Action of the new state */
                        BleConnectionStm_StateReconnectingDirectedLowExitAction(LinkId, normal);

                        gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedLow, IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));
                        state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    }
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp))
                {
                    BleConnectionStm_StateReconnectingDirectedLowExitAction(LinkId, abort);
                    gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_ReconnectDirectedLow, IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateStopReconnectingDirect, NULL, LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateReconnecting:
                /* Here state transitions and call to newState action should occur */
                if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnFail))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_CLEAR(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateBondedConnectedNoSecurityAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnOpen) && IS_CONF_SET(gpBlePeripheralConnectionStm_ConfigEncryptedOnly))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingExitAction(LinkId, normal);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity,BleConnectionStm_StateConnectedSlaveSecurityRequestAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                else if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp) || IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateReconnectingExitAction(LinkId, abort);

                    gpBlePeripheralConnectionStm_cbStopAdvertising(LinkId, gpBlePeripheralConnectionStm_Reconnect, IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseApp));

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateAbortReconnecting, BleConnectionStm_StateAbortReconnectingAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
            break;

            case gpBlePeripheralConnectionStm_StateAbortReconnecting:
                if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnFail))
                {
                    /* If state has exit action, first call the exit action before calling Action of the new state */
                    BleConnectionStm_StateAbortReconnectingExitAction(LinkId);

                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }

            break;

            case gpBlePeripheralConnectionStm_StateBLEHostInitialising:
                if (IS_EVENT(gpBlePeripheralConnectionStm_EventBLEHostResetComplete))
                {
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateIdle,NULL,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }

            break;

            case gpBlePeripheralConnectionStm_StateStopReconnectingDirect:
                if (IS_EVENT(gpBlePeripheralConnectionStm_EventConnCloseExt))
                {
                    BleConnectionStm_StateStopReconnectingDirectExitAction(LinkId);
                    state = BleConnectionStm(gpBlePeripheralConnectionStm_StateBondedNoConnection,BleConnectionStm_StateBondedNoConnectionAction,LinkId);
                    result = gpBlePeripheralConnectionStmResult_Success;
                }
                break;

            default:
                /* Ending up in this state is only possible if a memory corruption has occuered,
                   since it is unknown to assess the severity of the corruption, here an assert is
                   acceptable. */
                GP_ASSERT_SYSTEM(false);
        }

        /* Set new state to BleModule */
        linkState[LinkId] = state;

        if (result == gpBlePeripheralConnectionStmResult_NotProcessed)
        {
            GP_LOG_PRINTF("Event not processed on linkId %u, state is %u",0, LinkId, state);
        }
    }
    else
    {
        GP_LOG_PRINTF("Event not processed on linkId %u, link is invalid",0, LinkId);
    }

    return result;
/* </CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_SendEvent */
}
#if defined(GP_COMP_UNIT_TEST)
gpBlePeripheralConnectionStm_State_t gpBlePeripheralConnectionStm_GetState(UInt8 LinkId)
{
/* <CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_GetState */
    return linkState[LinkId];
/* </CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_GetState */
}
#endif //defined(GP_COMP_UNIT_TEST)
#if defined(GP_COMP_UNIT_TEST)
void gpBlePeripheralConnectionStm_SetState(gpBlePeripheralConnectionStm_State_t state, UInt8 LinkId)
{
/* <CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_SetState */
    BleDumpState(state, LinkId);
    linkState[LinkId] = state;
/* </CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_SetState */
}
#endif //defined(GP_COMP_UNIT_TEST)
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_SetStateConf(gpBlePeripheralConnectionStm_Config_t stateConf, UInt8 LinkId)
{
/* <CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_SetStateConf */
    gpBlePeripheralConnectionStm_Result_t result = gpBlePeripheralConnectionStmResult_NotProcessed;
    gpBlePeripheralConnectionStm_Config_t all =
        gpBlePeripheralConnectionStm_ConfigAutoRecon |
        gpBlePeripheralConnectionStm_ConfigEncryptedOnly |
        gpBlePeripheralConnectionStm_ConfigDirectedHighAdv |
        gpBlePeripheralConnectionStm_ConfigDirectedLowAdv |
        gpBlePeripheralConnectionStm_ConfigUndirectedAdv;
#ifdef GP_BLE_MULTI_ADV_SUPPORTED
    all |= gpBlePeripheralConnectionStm_ConfigPairingOnly;
#endif

    if (LinkId < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS && (stateConf & ~all) == 0)
    {
        linkStateConf[LinkId] = stateConf;
        result = gpBlePeripheralConnectionStmResult_Success;
    }

    return result;
/* </CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_SetStateConf */
}
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_IsAdvertising(UInt8 LinkId)
{
/* <CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_IsAdvertising */
    gpBlePeripheralConnectionStm_Result_t result = gpBlePeripheralConnectionStmResult_NotAdvertising;

    if (LinkId < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS)
    {
        gpBlePeripheralConnectionStm_State_t state = linkState[LinkId];

        if (state == gpBlePeripheralConnectionStm_StateConnecting || state == gpBlePeripheralConnectionStm_StateAbortConnecting ||
            state == gpBlePeripheralConnectionStm_StateReconnecting || state == gpBlePeripheralConnectionStm_StateAbortReconnecting ||
            state == gpBlePeripheralConnectionStm_StateReconnectingDirectedHigh ||
            state == gpBlePeripheralConnectionStm_StateReconnectingDirectedLow)
        {
            result = gpBlePeripheralConnectionStmResult_Success;
        }
    }

    return result;
/* </CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_IsAdvertising */
}
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_IsConnected(UInt8 LinkId)
{
/* <CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_IsConnected */
    gpBlePeripheralConnectionStm_Result_t result = gpBlePeripheralConnectionStmResult_NoConnection;

    if (LinkId < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS)
    {
        gpBlePeripheralConnectionStm_State_t state = linkState[LinkId];

        if (state == gpBlePeripheralConnectionStm_StateBondedConnected)
        {
            result = gpBlePeripheralConnectionStmResult_Success;
        }

        if (state == gpBlePeripheralConnectionStm_StateConnected)
        {
            result = gpBlePeripheralConnectionStmResult_ConnectedNoEncryption;
        }

        if (state == gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity)
        {
            result = gpBlePeripheralConnectionStmResult_BondedConnectedNoSecurity;
        }
    }

    return result;
/* </CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_IsConnected */
}
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_IsAbortConnecting(UInt8 LinkId)
{
/* <CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_IsAbortConnecting */
    gpBlePeripheralConnectionStm_Result_t result = gpBlePeripheralConnectionStmResult_NoConnection;

    if (LinkId < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS)
    {
        gpBlePeripheralConnectionStm_State_t state = linkState[LinkId];

        if (state == gpBlePeripheralConnectionStm_StateAbortConnecting || state == gpBlePeripheralConnectionStm_StateAbortReconnecting)
        {
            result = gpBlePeripheralConnectionStmResult_Success;
        }
    }

    return result;
/* </CodeGenerator Placeholder> Implementation_gpBlePeripheralConnectionStm_IsAbortConnecting */
}
