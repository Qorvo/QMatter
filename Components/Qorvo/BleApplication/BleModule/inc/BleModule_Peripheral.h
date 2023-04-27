/*
 *   Copyright (c) 2018, Qorvo Inc
 *
 *   BleModule is the interface to the host-stack. This file is intended to be modified by the customer according to the customers needs.
 *   Declarations of the public functions and enumerations of BleModule.
 *
 *   This software is owned by Qorvo Inc
 *   and protected under applicable copyright laws.
 *   It is delivered under the terms of the license
 *   and is intended and supplied for use solely and
 *   exclusively with products manufactured by
 *   Qorvo Inc.
 *
 *
 *   THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *   CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *   IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *   LIMITED TO, IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *   QORVO INC. SHALL NOT, IN ANY
 *   CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *   INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *   FOR ANY REASON WHATSOEVER.
 *
 *   $Header$
 *   $Change$
 *   $DateTime$
 */


#ifndef _BLEMODULE_PERIPHERAL_H_
#define _BLEMODULE_PERIPHERAL_H_

/// @file "BleModule.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "BleModule.h"
/* <CodeGenerator Placeholder> General */

/*! linkId none definition. Used for possibility to indicate unknown linkId */
#define BLE_MODULE_LINK_ID_NONE                                0xFF


#define IS_VALID_LINK_ID(linkId) (linkId < GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS)
#define BLE_MODULE_CURRENT_LINKID()   BleModule_CurrentLinkId
extern UInt8 BleModule_CurrentLinkId;

void BleModule_cbAttsCnf(attEvt_t *pMsg);
void BleModule_cbDmCnf(dmEvt_t *pMsg);

/* </CodeGenerator Placeholder> General */

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/** @enum BleModule_AdvConf_t */
//@{
/** @brief Undirected advertising is performed on reconnection */
#define BleModule_AdvConf_Undirected                             0x00
/** @brief Directed advertising with high duty cycle is performed on reconnection */
#define BleModule_AdvConf_Directed_HighDutyCycle                 0x01
/** @brief Directed advertising with low duty cycle is performed on reconnection */
#define BleModule_AdvConf_Directed_LowDutyCycle                  0x02
/** @typedef BleModule_AdvConf_t
    @brief Enum for configuring the advertisment done on reconnection
*/
typedef UInt8                             BleModule_AdvConf_t;
//@}

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define PROFILE_CONFIG_NONE         0
#define BEACON_SERVICE_ENABLED      1
#define AUTH_AND_BOND_FLAGS_INVALID 0xFF
#define USING_LTK_DATA_INVALID      false

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
/** @struct BleModule_Peripheral_Cb_t
 *  @brief Function pointers to the callbacks in the application
*/
typedef struct {
    /** @brief Open connection callback*/
    BleModule_AppCb_t                          openConnCb;
    /** @brief Close connection callback*/
    BleModule_AppCb_t                          closeConnCb;
    /** @brief Encryption callback*/
    BleModule_EncCb_t                          encCb;
    /** @brief Pairing callback*/
    BleModule_PairCb_t                         pairCb;
    /** @brief Security key exchanged callback*/
    BleModule_secKeyCb_t                       secKeyCb;
    /** @brief Unbind connection callback*/
    BleModule_AppCb_t                          unbindConnCb;
    /** @brief Update connection callback*/
    BleModule_UpdConnCb_t                      UpdateConnCb;
    /** @brief Advertising started callback*/
    BleModule_AppCb_t                          advStartedCb;
    /** @brief Authentication required callback*/
    BleModule_AuthReqCb_t                      authReqCb;
    /** @brief Address resolving configured callback */
    BleModule_AddressResCb_t                       addrResolvingCb;
} BleModule_Peripheral_Cb_t;

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
void BleModule_Peripheral_Init(void);

/** @brief Register the application callbacks for BleModule_Peripheral
*
*   @param BleModule_Peripheral_Cb_t    Pointer to struct of callback pointers for the Peripheral app
*/
void BleModule_Peripheral_RegisterCb(BleModule_Peripheral_Cb_t* peripheralCb);

/** @brief Create a new connection or restore a previously bonded link.
*
*   @param linkId                    linkId to use as reference to the new connection. Using a previously used linkId will trigger a reconnection with the previous bonding information.
*/
void BleModule_OpenConnectionRequest(UInt8 linkId);

/** @brief Close an existing connection.
*
*   @param linkId                    Reference of link to close.
*/
void BleModule_CloseConnectionRequest(UInt8 linkId);

/** @brief Remove all binding information of a bonded connection.
*
*   @param linkId                    Reference of link to remove bonding info for.
*/
void BleModule_UnbindConnectionRequest(UInt8 linkId);

/** @brief Send raw data over BLE link
*
*   @param linkId                    linkId to send data to.
*   @param length                    Length of data to send.
*   @param pData                     Data to send.
*/
void BleModule_SendDataRequest(UInt8 linkId, UInt16 length, UInt8* pData);

/** @brief One API to control the whitelist content
*
*   @param BleModule_WhiteListOp     Set operation to be performed on the whitelist
*   @param Length                    Number of linkId's which address needs to be added to / removed from the whitelist.
*   @param pLinkIdList               Pointer to array of linkId's to be added/removed
*   @return result                   return Success if operation could be performed, error code if no Success
*/
BleModule_Result_t BleModule_PopulateWhiteList(BleModule_WhitelistOp_t BleModule_WhiteListOp, UInt8 Length, UInt8* pLinkIdList);

/** @brief Called by application to start an application requested connection parameter update.
*
*   @param linkId                    LinkId to start connection parameter update request
*   @param pConPar                   Pointer to requested parameters
*                                    Note: UpdateConnCb is not called when @p pConPar is invalid or identical to the existing configuration.
*/
void BleModule_UpdateConnectionParametersRequest(UInt8 linkId, const BleModule_ConnectionParameters_t* pConPar);

//Indications
/** @brief Confirmation of stack reset done.
*/
void BleModule_cbResetDoneIndication(void);

/** @brief Confirm callback on DataRequest
*
*   @param linkId                    linkId data was sent to.
*   @param status                    status of the DataRequest
*/
void BleModule_cbSendDataConfirm(UInt8 linkId, BleModule_Result_t status);

/** @brief Set advertising timeout for specific Link ID
*
*   @param linkId                    Identifier for the link to be configured.
*   @param advTimeoutMS              The advertising duration in ms. Set to zero to advertise until stopped.
*/
void BleModule_SetAdvertisingTimeoutMs(UInt8 linkId, UInt16 advTimeoutMS);

/** @brief Set advertising interval for specific Link ID
*
*   @param linkId                    Identifier for the link to be configured.
*   @param advIntervalMs             The advertising interval in ms.
*/
void BleModule_SetAdvertisingIntervalMs(UInt8 linkId, UInt16 advIntervalMs);

/** @brief Set localName advertising parameter for specific Link ID
*
*   @param linkId                    Identifier for the link to be configured.
*   @param localName                 String containing the local name to be configured. Length and
*                                      local advertising name type is added by the function.
*/
void BleModule_SetLocalName(UInt8 linkId, char* localName);

/** @brief Check bonding state for specific Link ID
*
*   @param linkId                   Identifier for the link to check.
*/
Bool BleModule_IsBonded(UInt8 linkId);


/** @brief Check if connection is idle with SMP Pairing in progress
*
*   @param linkId                   Identifier for the link to check.
*   @return                         DM_CONN_ID_NONE if host is not idle in Smp Pairing
                                    Connection Id which has Smp Pairing in progress
*/
UInt8 BleModule_GetConnectionIdleWithSmpPairing(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_BLEMODULE_PERIPHERAL_H_
