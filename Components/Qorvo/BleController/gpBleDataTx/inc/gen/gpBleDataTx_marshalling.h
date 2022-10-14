/*
 * Copyright (c) 2019, Qorvo Inc
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

#ifndef _GPBLEDATATX_MARSHALLING_H_
#define _GPBLEDATATX_MARSHALLING_H_

//DOCUMENTATION BLEDATATX: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpBleDataTx.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/


typedef struct {
    Ble_IntConnId_t connId;
} gpBleDataTx_cbDataConfirm_Input_struct_t;

typedef struct {
    gpBleDataTx_cbDataConfirm_Input_struct_t data;
} gpBleDataTx_cbDataConfirm_Input_marshall_struct_t;


typedef struct {
    Ble_IntConnId_t connId;
} gpBle_TxResourceAvailableInd_Input_struct_t;

typedef struct {
    gpBle_TxResourceAvailableInd_Input_struct_t data;
} gpBle_TxResourceAvailableInd_Input_marshall_struct_t;


typedef struct {
    Ble_IntConnId_t connId;
} gpBle_DataTxOpenConnection_Input_struct_t;

typedef struct {
    gpBle_DataTxOpenConnection_Input_struct_t data;
} gpBle_DataTxOpenConnection_Input_marshall_struct_t;


typedef struct {
    Ble_IntConnId_t connId;
    Bool pause;
} gpBle_DataTxSetConnectionPause_Input_struct_t;

typedef struct {
    gpBle_DataTxSetConnectionPause_Input_struct_t data;
} gpBle_DataTxSetConnectionPause_Input_marshall_struct_t;


typedef struct {
    Ble_IntConnId_t connId;
} gpBle_DataTxCloseConnection_Input_struct_t;

typedef struct {
    gpBle_DataTxCloseConnection_Input_struct_t data;
} gpBle_DataTxCloseConnection_Input_marshall_struct_t;


typedef struct {
    Bool firstReset;
} gpBle_DataTxReset_Input_struct_t;

typedef struct {
    gpBle_DataTxReset_Input_struct_t data;
} gpBle_DataTxReset_Input_marshall_struct_t;


#if !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY))
typedef struct {
    gpHci_ConnectionHandle_t connHandle;
    UInt16 dataLength;
    UInt8* pData;
} gpBle_DataTxRequest_Input_struct_t;

typedef struct {
    gpBle_DataTxRequest_Input_struct_t data;
    UInt8 pData[GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_TX];
} gpBle_DataTxRequest_Input_marshall_struct_t;

#endif /* !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY)) */

typedef struct {
    Ble_IntConnId_t connId;
} gpBle_DataTxIsDataInBuffers_Input_struct_t;

typedef struct {
    gpBle_DataTxIsDataInBuffers_Input_struct_t data;
} gpBle_DataTxIsDataInBuffers_Input_marshall_struct_t;

typedef struct {
    Bool isDataInBuffers;
} gpBle_DataTxIsDataInBuffers_Output_struct_t;

typedef struct {
    gpBle_DataTxIsDataInBuffers_Output_struct_t data;
} gpBle_DataTxIsDataInBuffers_Output_marshall_struct_t;


typedef union {
    gpBleDataTx_cbDataConfirm_Input_marshall_struct_t gpBleDataTx_cbDataConfirm;
    gpBle_TxResourceAvailableInd_Input_marshall_struct_t gpBle_TxResourceAvailableInd;
    gpBle_DataTxOpenConnection_Input_marshall_struct_t gpBle_DataTxOpenConnection;
    gpBle_DataTxSetConnectionPause_Input_marshall_struct_t gpBle_DataTxSetConnectionPause;
    gpBle_DataTxCloseConnection_Input_marshall_struct_t gpBle_DataTxCloseConnection;
    gpBle_DataTxReset_Input_marshall_struct_t gpBle_DataTxReset;
#if !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY))
    gpBle_DataTxRequest_Input_marshall_struct_t gpBle_DataTxRequest;
#endif /* !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY)) */
    gpBle_DataTxIsDataInBuffers_Input_marshall_struct_t gpBle_DataTxIsDataInBuffers;
    UInt8 dummy; //ensure none empty union definition
} gpBleDataTx_Server_Input_union_t;

typedef union {
    gpBle_DataTxIsDataInBuffers_Output_marshall_struct_t gpBle_DataTxIsDataInBuffers;
    UInt8 dummy; //ensure none empty union definition
} gpBleDataTx_Server_Output_union_t;
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's

// Structure copy functions
// Server functions
gpMarshall_AckStatus_t gpBleDataTx_cbDataConfirm_Input_buf2api(gpBleDataTx_cbDataConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBle_TxResourceAvailableInd_Input_buf2api(gpBle_TxResourceAvailableInd_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBle_DataTxOpenConnection_Input_buf2api(gpBle_DataTxOpenConnection_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBle_DataTxSetConnectionPause_Input_buf2api(gpBle_DataTxSetConnectionPause_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBle_DataTxCloseConnection_Input_buf2api(gpBle_DataTxCloseConnection_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBle_DataTxReset_Input_buf2api(gpBle_DataTxReset_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
#if !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY))
gpMarshall_AckStatus_t gpBle_DataTxRequest_Input_buf2api(gpBle_DataTxRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY)) */
gpMarshall_AckStatus_t gpBle_DataTxIsDataInBuffers_Input_buf2api(gpBle_DataTxIsDataInBuffers_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpBle_DataTxIsDataInBuffers_Output_api2buf(UInt8Buffer* pDest , gpBle_DataTxIsDataInBuffers_Output_marshall_struct_t* pSourceoutput , gpBle_DataTxIsDataInBuffers_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);

// Client functions
void gpBleDataTx_cbDataConfirm_Input_par2buf(UInt8Buffer* pDest , Ble_IntConnId_t connId , UInt16* pIndex);
void gpBle_TxResourceAvailableInd_Input_par2buf(UInt8Buffer* pDest , Ble_IntConnId_t connId , UInt16* pIndex);
void gpBle_DataTxOpenConnection_Input_par2buf(UInt8Buffer* pDest , Ble_IntConnId_t connId , UInt16* pIndex);
void gpBle_DataTxSetConnectionPause_Input_par2buf(UInt8Buffer* pDest , Ble_IntConnId_t connId , Bool pause , UInt16* pIndex);
void gpBle_DataTxCloseConnection_Input_par2buf(UInt8Buffer* pDest , Ble_IntConnId_t connId , UInt16* pIndex);
void gpBle_DataTxReset_Input_par2buf(UInt8Buffer* pDest , Bool firstReset , UInt16* pIndex);
#if !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY))
void gpBle_DataTxRequest_Input_par2buf(UInt8Buffer* pDest , gpHci_ConnectionHandle_t connHandle , UInt16 dataLength , UInt8* pData , UInt16* pIndex);
#endif /* !(defined(GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY)) */
void gpBle_DataTxIsDataInBuffers_Input_par2buf(UInt8Buffer* pDest , Ble_IntConnId_t connId , UInt16* pIndex);
void gpBle_DataTxIsDataInBuffers_Output_buf2par(Bool* isDataInBuffers , Ble_IntConnId_t connId , UInt8Buffer* pSource , UInt16* pIndex);

void gpBleDataTx_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPBLEDATATX_MARSHALLING_H_


