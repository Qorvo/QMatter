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

/** @file "gpBlePeripheralConnectionStm.h"
 *
 *  State machine to create and maintain connections in a controlled way.
 *
 *  Declarations of the public functions and enumerations of gpBlePeripheralConnectionStm.
*/

#ifndef _GPBLEPERIPHERALCONNECTIONSTM_H_
#define _GPBLEPERIPHERALCONNECTIONSTM_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/** @enum gpBlePeripheralConnectionStm_Event_t */
//@{
/** @brief Event to trigger creation of a new Link or reopen an existing Link */
#define gpBlePeripheralConnectionStm_EventConnReq              0x00
/** @brief Claiming the advertising channel has failed (If Advertising Queue is properly configured, this event should never occur) */
#define gpBlePeripheralConnectionStm_EventAdvFail              0x01
/** @brief The Advertising channel is available */
#define gpBlePeripheralConnectionStm_EventAdvAvail             0x02
/** @brief An exiting connection is opened */
#define gpBlePeripheralConnectionStm_EventConnOpen             0x03
/** @brief A new connection is opened */
#define gpBlePeripheralConnectionStm_EventConnOpenNew          0x04
/** @brief Building a connection has failed */
#define gpBlePeripheralConnectionStm_EventConnFail             0x05
/** @brief Pairing (bonding) is completed */
#define gpBlePeripheralConnectionStm_EventPairComplete         0x06
/** @brief Legacy Event, to be removed */
#define gpBlePeripheralConnectionStm_EventNoPairRequest        0x07
/** @brief Connection close, triggered by application. There will be no attempt for automatic reconnection */
#define gpBlePeripheralConnectionStm_EventConnCloseApp         0x08
/** @brief Connection closed due to an event in the Host stack. If configured, there will be a reconnection attempt. */
#define gpBlePeripheralConnectionStm_EventConnCloseExt         0x09
/** @brief Building up security or start encryption has failed */
#define gpBlePeripheralConnectionStm_EventSecurityFail         0x0A
/** @brief The link is now encrypted */
#define gpBlePeripheralConnectionStm_EventSecurityEstablished     0x0B
/** @brief Pairing has failed */
#define gpBlePeripheralConnectionStm_EventPairFail             0x0C
/** @brief There is bond information available, this event should only be triggered after a reset of Ble */
#define gpBlePeripheralConnectionStm_EventBonded               0x0D
/** @brief BLE host reset complete */
#define gpBlePeripheralConnectionStm_EventBLEHostResetComplete     0x0E
/** @brief Connection unbind, triggered by application. */
#define gpBlePeripheralConnectionStm_EventUnbindApp            0x0F
/** @brief Event not used in code, defined to simplify unit-tests */
#define gpBlePeripheralConnectionStm_EventNumberOfEvents       0x10
/** @typedef gpBlePeripheralConnectionStm_Event_t
 *  @brief Definition of all events that act on the connection state machine
*/
typedef UInt8                             gpBlePeripheralConnectionStm_Event_t;
//@}

/** @enum gpBlePeripheralConnectionStm_State_t */
//@{
/** @brief Initialising, not ready to accept requests from application yet */
#define gpBlePeripheralConnectionStm_StateBLEHostInitialising     0x00
/** @brief Wait till advertisement channel is available */
#define gpBlePeripheralConnectionStm_StateWaitConnect          0x01
/** @brief Advertising to build a new connection */
#define gpBlePeripheralConnectionStm_StateConnecting           0x02
/** @brief Connecting aborted, wait for adv stopped */
#define gpBlePeripheralConnectionStm_StateAbortConnecting      0x03
/** @brief Connection open, pairing in progress */
#define gpBlePeripheralConnectionStm_StateConnected            0x04
/** @brief Connection open, pairing successful completed */
#define gpBlePeripheralConnectionStm_StateBondedConnected      0x05
/** @brief Not connected, valid pairing information available in database */
#define gpBlePeripheralConnectionStm_StateBondedNoConnection     0x06
/** @brief Connected, pairing information available but not encrypted */
#define gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity     0x07
/** @brief Wait till advertisement channel is available for reconnecting */
#define gpBlePeripheralConnectionStm_StateWaitReconnect        0x08
/** @brief Advertising to reconnect to a known device */
#define gpBlePeripheralConnectionStm_StateReconnecting         0x09
/** @brief Reconnecting aborted, wait for adv stopped */
#define gpBlePeripheralConnectionStm_StateAbortReconnecting     0x0A
/** @brief Advertising to reconnect to a known device (direct high duty cycle) */
#define gpBlePeripheralConnectionStm_StateReconnectingDirectedHigh     0x0B
/** @brief Advertising to reconnect to a known device (direct low duty cycle) */
#define gpBlePeripheralConnectionStm_StateReconnectingDirectedLow     0x0C
/** @brief No Ble activity in this state, connection closed if not already closed */
#define gpBlePeripheralConnectionStm_StateIdle                 0x0D
/** @brief Closing a connection while pairing is in progress */
#define gpBlePeripheralConnectionStm_StateClosingConnection     0x0E
/** @brief Closing a connection after pairing successfully completed */
#define gpBlePeripheralConnectionStm_StateBondedClosingConnection     0x0F
/** @brief Closing a connection after pairing successfully completed */
#define gpBlePeripheralConnectionStm_StateStopReconnectingDirect     0x10
/** @brief State not used, defined to simplify unit-tests */
#define gpBlePeripheralConnectionStm_StateNumberOfStates       0x11
/** @typedef gpBlePeripheralConnectionStm_State_t
 *  @brief Defining states the state machine can have.
*/
typedef UInt8                             gpBlePeripheralConnectionStm_State_t;
//@}

/** @enum gpBlePeripheralConnectionStm_AdvType_t */
//@{
/** @brief Start advertising for creating a new connection */
#define gpBlePeripheralConnectionStm_NewConnection             0x01
/** @brief Start adverting for reconnection */
#define gpBlePeripheralConnectionStm_Reconnect                 0x02
/** @brief Start adverting for reconnection */
#define gpBlePeripheralConnectionStm_ReconnectDirectedHigh     0x03
/** @brief Start adverting for reconnection */
#define gpBlePeripheralConnectionStm_ReconnectDirectedLow      0x04
/** @typedef gpBlePeripheralConnectionStm_AdvType_t
 *  @brief Enum to request advertising for building a new connection or reconnecting an exiting connection.
*/
typedef UInt8                             gpBlePeripheralConnectionStm_AdvType_t;
//@}

/** @enum gpBlePeripheralConnectionStm_Config_t */
//@{
/** @brief Mask for testing configuration not set */
#define gpBlePeripheralConnectionStm_ConfigCleared             0x00
/** @brief Config Auto reconnect is set */
#define gpBlePeripheralConnectionStm_ConfigAutoRecon           0x01
/** @brief Config Only Encrypted connection is set */
#define gpBlePeripheralConnectionStm_ConfigEncryptedOnly       0x02
/** @brief Config Directed high duty cycle advertisement on reconnect */
#define gpBlePeripheralConnectionStm_ConfigDirectedHighAdv     0x04
/** @brief Config Directed low duty cycle advertisement on reconnect */
#define gpBlePeripheralConnectionStm_ConfigDirectedLowAdv      0x08
/** @brief Config Undirected advertisement on reconnect */
#define gpBlePeripheralConnectionStm_ConfigUndirectedAdv       0x10
#ifdef GP_BLE_MULTI_ADV_SUPPORTED
/** @brief Config Pairing only(no bonding) is set */
#define gpBlePeripheralConnectionStm_ConfigPairingOnly         0x20
#endif
/** @typedef gpBlePeripheralConnectionStm_Config_t
 *  @brief Defining the config flags for state transitions. Note, this are bit-wise assignments
*/
typedef UInt8                             gpBlePeripheralConnectionStm_Config_t;
//@}

/** @enum gpBlePeripheralConnectionStm_Result_t */
//@{
/** @brief Result success, is returned in case of successful transaction or the result of a status request function is positive. */
#define gpBlePeripheralConnectionStmResult_Success             0x00
/** @brief gpBlePeripheralConnectionStm, no connection available */
#define gpBlePeripheralConnectionStmResult_NoConnection        0x01
/** @brief There is a link but the link is not encrypted */
#define gpBlePeripheralConnectionStmResult_ConnectedNoEncryption     0x02
/** @brief gpBlePeripheralConnectionStm, the state-machine is not in an advertising state */
#define gpBlePeripheralConnectionStmResult_NotAdvertising      0x03
/** @brief gpBlePeripheralConnectionStm, Event is not processed. This occurs when state machine is in a state where received event has no effect. */
#define gpBlePeripheralConnectionStmResult_NotProcessed        0x04
/** @brief There is a bond but encryption is missing. */
#define gpBlePeripheralConnectionStmResult_BondedConnectedNoSecurity     0x05
/** @brief An unexpected result from a requested action/event */
#define gpBlePeripheralConnectionStmResult_UnknownError        0xFF
/** @typedef gpBlePeripheralConnectionStm_Result_t
 *  @brief Result types for the gpBlePeripheralConnectionStm
*/
typedef UInt8                             gpBlePeripheralConnectionStm_Result_t;
//@}

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
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
/** @brief Send an event to the Connection statemachine. Inside the scope of the statemachine, actions to be triggered will be started.
*
*   @param Event                     Event to be processed
*   @param LinkId                    Id of active link
*   @return result                   Returns gpResult_Success if event is processed. If statemachine is in a state where event takes no effect gpResult_NotProcessed will be returned.
*/
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_SendEvent(gpBlePeripheralConnectionStm_Event_t Event, UInt8 LinkId);

#if defined(GP_COMP_UNIT_TEST)
/** @brief Get the state of the given link.
*
*   @param LinkId                    Id of link to be inquired
*   @return result                   State of the link.
*/
gpBlePeripheralConnectionStm_State_t gpBlePeripheralConnectionStm_GetState(UInt8 LinkId);
#endif //defined(GP_COMP_UNIT_TEST)

#if defined(GP_COMP_UNIT_TEST)
/** @brief Set the state configuraton for the given link.
*
*   @param state                     State configuration to be set
*   @param LinkId                    Id of link to be configured
*/
void gpBlePeripheralConnectionStm_SetState(gpBlePeripheralConnectionStm_State_t state, UInt8 LinkId);
#endif //defined(GP_COMP_UNIT_TEST)

/** @brief Set the state configuraton for the given link.
*
*   @param stateConf                 State configuration to be set
*   @param LinkId                    Id of link to be configured
*   @return result                   Returns gpResult_Success if both linkId and stateConf are valid otherwise gpResult_NotProcessed.
*/
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_SetStateConf(gpBlePeripheralConnectionStm_Config_t stateConf, UInt8 LinkId);

/** @brief Function to request if there is active advertising for LinkId
*
*   @param LinkId                    Id of active link
*   @return result                   Returns gpBlePeripheralConnectionStmResult_Success if advertising is active, if not gpBlePeripheralConnectionStmResult_NotAdvertising will be returned
*/
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_IsAdvertising(UInt8 LinkId);

/** @brief Function to request if current connection is in connected state.
*
*   @param LinkId                    Id of active link
*   @return result                   Returns gpResult_Success if statemachine in state gpBlePeripheralConnectionStm_StateBondedConnected. If in state gpBlePeripheralConnectionStm_StateConnected or gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity gpBlePeripheralConnectionStmResult_ConnectedNoEncryption will be returned, in all other states it will returngpResult_NoConnection
*/
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_IsConnected(UInt8 LinkId);

/** @brief Funtion to check the connection is aborting.
*
*   @param LinkId                    Id of active link
*   @return result                   Returns gpResult_Success if statemachine in state gpBlePeripheralConnectionStm_StateBondedConnected. If in state gpBlePeripheralConnectionStm_StateConnected or gpBlePeripheralConnectionStm_StateBondedConnectedNoSecurity gpBlePeripheralConnectionStm_ConnectedNoEncryption will be returned, in all other states it will returngpResult_NoConnection
*/
gpBlePeripheralConnectionStm_Result_t gpBlePeripheralConnectionStm_IsAbortConnecting(UInt8 LinkId);

//Indications
/** @brief Request to start advertising for LinkId
*
*   @param LinkId                    Id of active link
*   @param AdvType                   Advertise for new connection or advertise for reconnection
*/
void gpBlePeripheralConnectionStm_cbStartAdvertising(UInt8 LinkId, gpBlePeripheralConnectionStm_AdvType_t AdvType);

/** @brief Request to stop advertising for LinkId
*
*   @param LinkId                    Id of active link
*   @param AdvType                   Type of the advertisement being stopped
*   @param disconnectingRequested    Indicate if the advertisement stop is triggered via a disconnection request (true) or an internal advertisement timeout (false)
*/
void gpBlePeripheralConnectionStm_cbStopAdvertising(UInt8 LinkId, gpBlePeripheralConnectionStm_AdvType_t AdvType, Bool disconnectingRequested);

/** @brief Request to close connection with LinkId
*
*   @param LinkId                    Id of active link
*/
void gpBlePeripheralConnectionStm_cbCloseConnection(UInt8 LinkId);

/** @brief Request to start security timeout. When security is not established after the timeout a Slave Security Request will be sent.
*
*   @param LinkId                    Id of active link
*/
void gpBlePeripheralConnectionStm_cbScheduleSecurityTimeout(UInt8 LinkId);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPBLEPERIPHERALCONNECTIONSTM_H_

