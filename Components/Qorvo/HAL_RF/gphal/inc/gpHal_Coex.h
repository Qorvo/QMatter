/*
 * Copyright (c) 2017-2019, Qorvo Inc
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
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 * $Header$
 * $Change$
 * $DateTime$
 */

/** @file "gpHal_Coex.h"
 *
 *  gpHal Coexistence subcomponent
 *
 *  Declarations of the public functions and enumerations of gpHal_Coex.
*/

#ifndef _GPHAL_COEX_H_
#define _GPHAL_COEX_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gp_global.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/** @enum gpHal_MAC_ReRequestTrigger_t */
//@{
/** @brief No action. */
#define gpHal_MAC_ReRequestTrigger_None                        0x00
/** @brief Re-Request if grant is lost or not given. */
#define gpHal_MAC_ReRequestTrigger_NoGrant                     0x01
/** @brief Re-Request on priority change during ongoing request. */
#define gpHal_MAC_ReRequestTrigger_PrioChange                  0x02
/** @typedef gpHal_MAC_ReRequestTrigger_t
    @brief Mask with the ReRequest triggers.
*/
typedef UInt8                             gpHal_MAC_ReRequestTrigger_t;
//@}

/** @enum gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t */
//@{
/** @brief No action, grant not aware. */
#define gpHal_MAC_TX_Packet_Ignore                             0x00
/** @brief If not granted, then disable PA */
#define gpHal_Coex_MAC_TX_Packet_DisablePa                     0x01
/** @brief If not granted, then trigger a CMSA_CA failure. Only applicable to TX_packet. */
#define gpHal_Coex_MAC_TX_Packet_CcaFailure                    0x02
/** @brief If not granted, CSMA-CA delay backoff counter get frozen. Once grant is received, conter proceeds. Can be used to force wait for grant before CCA measurement. When using SW CSMA-CA, backoff delay is handled by SW, so this option is ignored. */
#define gpHal_Coex_MAC_TX_Packet_CcaHold                       0x04
/** @brief If not granted, TX state machine is hold after doing CSMA-CA, but before starting the TX. Once Grant is received, TX starts imediatelly. */
#define gpHal_Coex_MAC_TX_Packet_DelayedStart                  0x08
/** @typedef gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t
    @brief Mask that sets the MAC packet TX actions upon not having grant.
*/
typedef UInt8                             gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t;
//@}

/** @enum gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t */
//@{
/** @brief No action, grant not aware. */
#define gpHal_MAC_TX_ACK_Ignore                                0x00
/** @brief If not granted, then disable PA */
#define gpHal_Coex_MAC_TX_ACK_DisablePa                        0x01
/** @brief If not granted, skip sending of the ACK. */
#define gpHal_Coex_MAC_TX_ACK_SkipTx                           0x02
/** @typedef gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t
    @brief Mask that sets the MAC ACK TX actions upon not having grant.
*/
typedef UInt8                             gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t;
//@}

/** @enum gpHal_MAC_ReqExtTrigger_t */
//@{
/** @brief No extension. */
#define gpHal_MAC_ExtensionTriggers_None                       0x00
/** @brief Extend on preamble detect */
#define gpHal_MAC_ExtensionTriggers_Preamble                   0x01
/** @brief Extend on SFD reception */
#define gpHal_MAC_ExtensionTriggers_SFD                        0x02
/** @brief Extend on macfilter */
#define gpHal_MAC_ExtensionTriggers_PacketAbort                0x04
/** @brief Extend on CRC failure */
#define gpHal_MAC_ExtensionTriggers_FCSERR                     0x08
/** @typedef gpHal_MAC_ReqExtTrigger_t
    @brief Mask that sets request extension triggers
*/
typedef UInt8                             gpHal_MAC_ReqExtTrigger_t;
//@}

/** @enum gpHal_GainControl_Mode_t */
//@{
/** @brief  Use default fixed gain control level LNA0 */
#define gpHal_GainControl_Mode_Default                         0x00
/** @brief Gain control levels will be fixed to the specified lowLnaAtt setting */
#define gpHal_GainControl_Mode_Fixed                           0x01
/** @brief Gain control levels will be controlled by the internal AGC */
#define gpHal_GainControl_Mode_RssiBasedAgc                    0x02
/** @brief Gain control levels will be controlled by the configured COEX_ATT_CTRL BSP setting */
#define gpHal_GainControl_Mode_GpioBasedAgc                    0x03
/** @typedef gpHal_GainControl_Mode_t
    @brief Enumeration specifying the gain control mode
*/
typedef UInt8                             gpHal_GainControl_Mode_t;
//@}

/** @enum gpHal_AttLna_t */
//@{
/** @brief Select LNA0 */
#define gpHal_AttLna_LNA0                                      0x00
/** @brief Select LNA1 */
#define gpHal_AttLna_LNA1                                      0x01
/** @brief Select LNA2 */
#define gpHal_AttLna_LNA2                                      0x02
/** @brief Select LNA3 */
#define gpHal_AttLna_LNA3                                      0x03
/** @brief Select LNA4 */
#define gpHal_AttLna_LNA4                                      0x04
/** @brief Select LNA5 */
#define gpHal_AttLna_LNA5                                      0x05
/** @brief Don't use/update LNA setting */
#define gpHal_AttLna_Ignore                                    0xFF
/** @typedef gpHal_AttLna_t
    @brief Selection of predefined LNA condif
*/
typedef UInt8                             gpHal_AttLna_t;
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

/* Callbacks for data confirm and GPIO interrupt */
typedef void (*gpHal_CoexCbDataConfirm_t)(UInt8 result, UInt8 retries, UInt8 priority);
typedef UInt8 (*gpHal_CoexCbGpioInt_t)(UInt8 interruptsMasked);
typedef void (*gpHal_CoexCbCSMARetry_t)(UInt8 result, void *pCSMA_CA_State);

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
/** @brief Set the coexistence parameters for 802.15.4 packet RX
*
*   @param request
*   @param priority
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_RX_Packet(Bool request, UInt8 priority);

/** @brief Set the coexistence parameters for 802.15.4 ACK TX
*
*   @param request
*   @param priority
*   @param txAckNotGrantedAction
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_TX_ACK(Bool request, UInt8 priority, gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t txAckNotGrantedAction);

/** @brief Set the coexistence parameters for 802.15.4 packet TX
*
*   @param request
*   @param priority
*   @param txNotGrantedAction
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_TX_Packet(Bool request, UInt8 priority, gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t txNotGrantedAction);

/** @brief Set the coexistence parameters for 802.15.4 ACK RX
*
*   @param request
*   @param priority
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_RX_ACK(Bool request, UInt8 priority);

/** @brief Configure the request extensions
*
*   @param trigger
*   @param priority
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_RX_ReqExt(gpHal_MAC_ReqExtTrigger_t trigger, UInt8 priority);

/** @brief Set the gain control
*
*   @param gainControlMode
*   @param attLnaLow
*   @param attLnaHigh
*   @return result
*/
gpHal_Result_t gpHal_Set_GainControl(gpHal_GainControl_Mode_t gainControlMode, gpHal_AttLna_t attLnaLow, gpHal_AttLna_t attLnaHigh);

/** @brief Enable early preamble detection
*
*   @param enableEarlyPreambleDetect
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_EarlyPreambleDetect(Bool enableEarlyPreambleDetect);

/** @brief Set extension timeout in multiples of 16 us.
*
*   @param extCoexTimeout
*   @return result                   Returns false when value out of range, else sucess.
*/
gpHal_Result_t gpHal_Set_MAC_ExtensionTimeout(UInt32 extCoexTimeout);

/** @brief Set the number of consecutive mac retries before raising the priority of the packet TX. Setting to 0 deactivate it
*
*   @param retriesCnt
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_MacRetriesTreshold(UInt8 retriesCnt);

/** @brief Set the number of consecutive cca retries before raising the priority of the packet TX. Setting to 0 deactivate it
*
*   @param retriesCnt
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_CcaRetriesTreshold(UInt8 retriesCnt);

/** @brief Configure if the request line should be re-requested on case of grant not given or lost for over x time
*
*   @param trigger                   Enable re-request upon no-grant or priority change
*   @param offTime                   Set the time in uS that request should be off during a re-request toggle.
*   @param onTime                    Set the time in uS it should wait for the grant, before turning request off to turn it on again.
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_ReRequest(gpHal_MAC_ReRequestTrigger_t trigger, UInt8 offTime, UInt8 onTime);

/** @brief Configure if an 802.15.4 indirect packet TX should have a different priority level as normal TX.
*
*   @param enable                    Enable the priority boost for indirect TX.
*                                       When enabled, all indirect TX will use the priority defined by this API.
*                                       When disabled, the priority applied is the same on for a normal packet TX.
*   @param priority                  Priority level to be used when enabled.
*   @return result
*/
gpHal_Result_t gpHal_Set_MAC_IndTxPriorityBoost(Bool enable, UInt8 priority);

//Indications
/** @brief Finalize handling after timeout on "Wait for COEX GRANT"
*   Actions:
*       Restore saved configuration for PA (internal and, if applcable, external PA)
*       Modify result accordingly
*
*/
UInt8 gpHal_CompletePossibleGrantimeoutByConfigRestore(UInt8 result);
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPHAL_COEX_H_

