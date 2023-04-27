/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   BleModule_Dat is the interface Dat functionalities. This file is intended to be modified by the customer according to the customers needs.
 *   Declarations of the public functions and enumerations of BleModule_Dat.
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


#ifndef _BLEMODULE_DATS_H_
#define _BLEMODULE_DATS_H_

/// @file "BleModule_Dats.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "BleModule.h"
#include "svc_wp.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/** @macro GP_BLEMODULE_LINKID_BROADCAST */
/** @brief Special link ID value to Tx to all connected devices. */
#define GP_BLEMODULE_LINKID_BROADCAST                0xFE

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
/** @brief Request to send raw data over BLE link
*
*   @param linkId                    linkId to send data to. Use GP_BLEMODULE_LINKID_BROADCAST to send to all connections.
*   @param length                    Length of data to send.
*   @param pData                     Data to send.
*/
void BleModule_DatDataRequest(UInt8 linkId, UInt16 length, UInt8* pData);


//Indications
/** @brief Confirm callback on DataRequest
*
*   @param linkId                    linkId data was sent to.
*   @param status                    Either BleModule_Success on success or the failure reason
*/
void BleModule_cbDatDataConfirm(UInt8 linkId, BleModule_Result_t status);

/** @brief Callback from host which prints the new data received.
*
*   @param linkId                    linkId received data.
*   @param length                    Length of received data.
*   @param pData                     Data received.
*   @param status                    Either BleModule_Success on success or the failure reason
*/
void BleModule_cbDatDataIndication(UInt8 linkId, UInt16 length, UInt8* pData, BleModule_Result_t status);

//Indications: Internal usage (in BleModule)
/**@brief Callback to indicate proprietary data is written
*
*   @param connId
*   @param handle
*   @param operation
*   @param offset
*   @param len
*   @param pValue
*   @param pAttr
*/
uint8_t BleModule_DatWpWriteCback(dmConnId_t connId, UInt16 handle, UInt8 operation,
                               UInt16 offset, UInt16 len, UInt8* pValue, attsAttr_t* pAttr);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_BLEMODULE_DATS_H_

