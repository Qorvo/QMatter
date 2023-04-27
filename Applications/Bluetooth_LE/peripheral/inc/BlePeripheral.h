/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   BLE Peripheral API
 *   Declarations of the public functions and enumerations of BLE Peripheral.
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


#ifndef _BLEPERIPHERAL_H_
#define _BLEPERIPHERAL_H_

/// @file "BlePeripheral.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#define Ble_StatusSuccess                       0x0
#define Ble_StatusLinkIdInvalid                 0x1
#define Ble_StatusDisconnected                  0x2
#define Ble_StatusRequestNotProcessed           0x9
#define Ble_StatusUnexpectedLinkOpenLinkDropped 0xA
#define Ble_StatusOpenFailNoResponse            0xB
typedef UInt8 Ble_Status_t;

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
/** @brief Initialize the BLE Peripheral
*/
void BlePeripheral_Init(void (*startConfirm)(Ble_Status_t), void (*dataIndication)(UInt8, UInt16, UInt8*));

/** @brief Starts the Ble stack and attempts a connection setup
 *
 *  @param linkId Identifier of the link.
 *
*/
void BlePeripheral_Start(UInt8 linkId);

/** @brief To be called to send data over the BLE service
*
*   @param linkId Identifier of the link.
*   @param length Length of data buffer to send.
*   @param pData  Pointer to data buffer to send.
*/
void BlePeripheral_DataRequest(UInt8 linkId, UInt16 length, UInt8* pData);

/** @brief Close connection
*
*   @param linkId Identifier of the link.
*/
void BlePeripheral_CloseConnection(UInt8 linkId);

/** @brief Close all opened connections - called before factory reset
*/
void BlePeripheral_CloseConnections(void);

/** @brief Unbind connection
*
*   @param linkId Identifier of the link.
*/
void BlePeripheral_UnbindConnection(UInt8 linkId);

/** @brief Clear all binding information - called in factory reset
*/
void BlePeripheral_FactoryReset(void);

/** @brief Dump the BLE connection information
*/
void BlePeripheral_DumpConnInfo(void);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_BLEPERIPHERAL_H_
