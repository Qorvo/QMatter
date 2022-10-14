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
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 * $Header$
 * $Change$
 * $DateTime$
 */

#ifndef _GPHAL_COEX_MARSHALLING_H_
#define _GPHAL_COEX_MARSHALLING_H_

//DOCUMENTATION HAL_COEX: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpHal_Coex.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    Bool request;
    UInt8 priority;
} gpHal_Set_MAC_RX_Packet_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_RX_Packet_Input_struct_t data;
} gpHal_Set_MAC_RX_Packet_Input_marshall_struct_t;


typedef struct {
    Bool request;
    UInt8 priority;
    gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t txAckNotGrantedAction;
} gpHal_Set_MAC_TX_ACK_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_TX_ACK_Input_struct_t data;
} gpHal_Set_MAC_TX_ACK_Input_marshall_struct_t;


typedef struct {
    Bool request;
    UInt8 priority;
    gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t txNotGrantedAction;
} gpHal_Set_MAC_TX_Packet_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_TX_Packet_Input_struct_t data;
} gpHal_Set_MAC_TX_Packet_Input_marshall_struct_t;


typedef struct {
    Bool request;
    UInt8 priority;
} gpHal_Set_MAC_RX_ACK_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_RX_ACK_Input_struct_t data;
} gpHal_Set_MAC_RX_ACK_Input_marshall_struct_t;


typedef struct {
    gpHal_MAC_ReqExtTrigger_t trigger;
    UInt8 priority;
} gpHal_Set_MAC_RX_ReqExt_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_RX_ReqExt_Input_struct_t data;
} gpHal_Set_MAC_RX_ReqExt_Input_marshall_struct_t;


typedef struct {
    gpHal_GainControl_Mode_t gainControlMode;
    gpHal_AttLna_t attLnaLow;
    gpHal_AttLna_t attLnaHigh;
} gpHal_Set_GainControl_Input_struct_t;

typedef struct {
    gpHal_Set_GainControl_Input_struct_t data;
} gpHal_Set_GainControl_Input_marshall_struct_t;

typedef struct {
    Bool result;
} gpHal_Set_GainControl_Output_struct_t;

typedef struct {
    gpHal_Set_GainControl_Output_struct_t data;
} gpHal_Set_GainControl_Output_marshall_struct_t;


typedef struct {
    Bool enableEarlyPreambleDetect;
} gpHal_Set_MAC_EarlyPreambleDetect_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_EarlyPreambleDetect_Input_struct_t data;
} gpHal_Set_MAC_EarlyPreambleDetect_Input_marshall_struct_t;


typedef struct {
    UInt32 extCoexTimeout;
} gpHal_Set_MAC_ExtensionTimeout_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_ExtensionTimeout_Input_struct_t data;
} gpHal_Set_MAC_ExtensionTimeout_Input_marshall_struct_t;

typedef struct {
    Bool result;
} gpHal_Set_MAC_ExtensionTimeout_Output_struct_t;

typedef struct {
    gpHal_Set_MAC_ExtensionTimeout_Output_struct_t data;
} gpHal_Set_MAC_ExtensionTimeout_Output_marshall_struct_t;


typedef struct {
    UInt8 retriesCnt;
} gpHal_Set_MAC_MacRetriesTreshold_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_MacRetriesTreshold_Input_struct_t data;
} gpHal_Set_MAC_MacRetriesTreshold_Input_marshall_struct_t;


typedef struct {
    UInt8 retriesCnt;
} gpHal_Set_MAC_CcaRetriesTreshold_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_CcaRetriesTreshold_Input_struct_t data;
} gpHal_Set_MAC_CcaRetriesTreshold_Input_marshall_struct_t;


typedef struct {
    gpHal_MAC_ReRequestTrigger_t trigger;
    UInt8 offTime;
    UInt8 onTime;
} gpHal_Set_MAC_ReRequest_Input_struct_t;

typedef struct {
    gpHal_Set_MAC_ReRequest_Input_struct_t data;
} gpHal_Set_MAC_ReRequest_Input_marshall_struct_t;


typedef union {
    gpHal_Set_MAC_RX_Packet_Input_marshall_struct_t gpHal_Set_MAC_RX_Packet;
    gpHal_Set_MAC_TX_ACK_Input_marshall_struct_t gpHal_Set_MAC_TX_ACK;
    gpHal_Set_MAC_TX_Packet_Input_marshall_struct_t gpHal_Set_MAC_TX_Packet;
    gpHal_Set_MAC_RX_ACK_Input_marshall_struct_t gpHal_Set_MAC_RX_ACK;
    gpHal_Set_MAC_RX_ReqExt_Input_marshall_struct_t gpHal_Set_MAC_RX_ReqExt;
    gpHal_Set_GainControl_Input_marshall_struct_t gpHal_Set_GainControl;
    gpHal_Set_MAC_EarlyPreambleDetect_Input_marshall_struct_t gpHal_Set_MAC_EarlyPreambleDetect;
    gpHal_Set_MAC_ExtensionTimeout_Input_marshall_struct_t gpHal_Set_MAC_ExtensionTimeout;
    gpHal_Set_MAC_MacRetriesTreshold_Input_marshall_struct_t gpHal_Set_MAC_MacRetriesTreshold;
    gpHal_Set_MAC_CcaRetriesTreshold_Input_marshall_struct_t gpHal_Set_MAC_CcaRetriesTreshold;
    gpHal_Set_MAC_ReRequest_Input_marshall_struct_t gpHal_Set_MAC_ReRequest;
    UInt8 dummy; //ensure none empty union definition
} gpHal_Coex_Server_Input_union_t;

typedef union {
    gpHal_Set_GainControl_Output_marshall_struct_t gpHal_Set_GainControl;
    gpHal_Set_MAC_ExtensionTimeout_Output_marshall_struct_t gpHal_Set_MAC_ExtensionTimeout;
    UInt8 dummy; //ensure none empty union definition
} gpHal_Coex_Server_Output_union_t;
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpHal_MAC_ReRequestTrigger_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpHal_MAC_ReRequestTrigger_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpHal_MAC_ReRequestTrigger_t_buf2api_1(pDest, pSource, pIndex) UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpHal_MAC_ReRequestTrigger_t_api2buf_1(pDest, pSource, pIndex) UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t_buf2api_1(pDest, pSource, pIndex) UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t_api2buf_1(pDest, pSource, pIndex) UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t_buf2api_1(pDest, pSource, pIndex) UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t_api2buf_1(pDest, pSource, pIndex) UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpHal_MAC_ReqExtTrigger_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpHal_MAC_ReqExtTrigger_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpHal_MAC_ReqExtTrigger_t_buf2api_1(pDest, pSource, pIndex) UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpHal_MAC_ReqExtTrigger_t_api2buf_1(pDest, pSource, pIndex) UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpHal_GainControl_Mode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpHal_GainControl_Mode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpHal_GainControl_Mode_t_buf2api_1(pDest, pSource, pIndex) UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpHal_GainControl_Mode_t_api2buf_1(pDest, pSource, pIndex) UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpHal_AttLna_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpHal_AttLna_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpHal_AttLna_t_buf2api_1(pDest, pSource, pIndex) UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpHal_AttLna_t_api2buf_1(pDest, pSource, pIndex) UInt8_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
// Server functions
gpMarshall_AckStatus_t gpHal_Set_MAC_RX_Packet_Input_buf2api( gpHal_Set_MAC_RX_Packet_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_TX_ACK_Input_buf2api( gpHal_Set_MAC_TX_ACK_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_TX_Packet_Input_buf2api( gpHal_Set_MAC_TX_Packet_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_RX_ACK_Input_buf2api( gpHal_Set_MAC_RX_ACK_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_RX_ReqExt_Input_buf2api( gpHal_Set_MAC_RX_ReqExt_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_GainControl_Input_buf2api( gpHal_Set_GainControl_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
void gpHal_Set_GainControl_Output_api2buf( UInt8Buffer* pDest , gpHal_Set_GainControl_Output_marshall_struct_t* pSourceoutput , gpHal_Set_GainControl_Input_marshall_struct_t* pSourceinput , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_EarlyPreambleDetect_Input_buf2api( gpHal_Set_MAC_EarlyPreambleDetect_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_ExtensionTimeout_Input_buf2api( gpHal_Set_MAC_ExtensionTimeout_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
void gpHal_Set_MAC_ExtensionTimeout_Output_api2buf( UInt8Buffer* pDest , gpHal_Set_MAC_ExtensionTimeout_Output_marshall_struct_t* pSourceoutput , gpHal_Set_MAC_ExtensionTimeout_Input_marshall_struct_t* pSourceinput , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_MacRetriesTreshold_Input_buf2api( gpHal_Set_MAC_MacRetriesTreshold_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_CcaRetriesTreshold_Input_buf2api( gpHal_Set_MAC_CcaRetriesTreshold_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );
gpMarshall_AckStatus_t gpHal_Set_MAC_ReRequest_Input_buf2api( gpHal_Set_MAC_ReRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );

// Client functions
void gpHal_Set_MAC_RX_Packet_Input_par2buf( UInt8Buffer* pDest , Bool request , UInt8 priority , UInt16* pIndex );
void gpHal_Set_MAC_TX_ACK_Input_par2buf( UInt8Buffer* pDest , Bool request , UInt8 priority , gpHal_Coex_MAC_TX_ACK_NotGrantedActions_t txAckNotGrantedAction , UInt16* pIndex );
void gpHal_Set_MAC_TX_Packet_Input_par2buf( UInt8Buffer* pDest , Bool request , UInt8 priority , gpHal_Coex_MAC_TX_Packet_NotGrantedActions_t txNotGrantedAction , UInt16* pIndex );
void gpHal_Set_MAC_RX_ACK_Input_par2buf( UInt8Buffer* pDest , Bool request , UInt8 priority , UInt16* pIndex );
void gpHal_Set_MAC_RX_ReqExt_Input_par2buf( UInt8Buffer* pDest , gpHal_MAC_ReqExtTrigger_t trigger , UInt8 priority , UInt16* pIndex );
void gpHal_Set_GainControl_Input_par2buf( UInt8Buffer* pDest , gpHal_GainControl_Mode_t gainControlMode , gpHal_AttLna_t attLnaLow , gpHal_AttLna_t attLnaHigh , UInt16* pIndex );
void gpHal_Set_GainControl_Output_buf2par( Bool* result , gpHal_GainControl_Mode_t gainControlMode , gpHal_AttLna_t attLnaLow , gpHal_AttLna_t attLnaHigh , UInt8Buffer* pSource , UInt16* pIndex );
void gpHal_Set_MAC_EarlyPreambleDetect_Input_par2buf( UInt8Buffer* pDest , Bool enableEarlyPreambleDetect , UInt16* pIndex );
void gpHal_Set_MAC_ExtensionTimeout_Input_par2buf( UInt8Buffer* pDest , UInt32 extCoexTimeout , UInt16* pIndex );
void gpHal_Set_MAC_ExtensionTimeout_Output_buf2par( Bool* result , UInt32 extCoexTimeout , UInt8Buffer* pSource , UInt16* pIndex );
void gpHal_Set_MAC_MacRetriesTreshold_Input_par2buf( UInt8Buffer* pDest , UInt8 retriesCnt , UInt16* pIndex );
void gpHal_Set_MAC_CcaRetriesTreshold_Input_par2buf( UInt8Buffer* pDest , UInt8 retriesCnt , UInt16* pIndex );
void gpHal_Set_MAC_ReRequest_Input_par2buf( UInt8Buffer* pDest , gpHal_MAC_ReRequestTrigger_t trigger , UInt8 offTime , UInt8 onTime , UInt16* pIndex );

void gpHal_Coex_InitMarshalling( void );

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPHAL_COEX_MARSHALLING_H_


