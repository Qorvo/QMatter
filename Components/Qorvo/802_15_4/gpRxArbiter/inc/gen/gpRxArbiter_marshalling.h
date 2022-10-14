/*
 * Copyright (c) 2015, GreenPeak Technologies
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

#ifndef _GPRXARBITER_MARSHALLING_H_
#define _GPRXARBITER_MARSHALLING_H_

//DOCUMENTATION RXARBITER: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpRxArbiter.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/


typedef struct {
    gpRxArbiter_StackId_t stackId;
} gpRxArbiter_ResetStack_Input_struct_t;

typedef struct {
    gpRxArbiter_ResetStack_Input_struct_t data;
} gpRxArbiter_ResetStack_Input_marshall_struct_t;

typedef struct {
    gpRxArbiter_Result_t result;
} gpRxArbiter_ResetStack_Output_struct_t;

typedef struct {
    gpRxArbiter_ResetStack_Output_struct_t data;
} gpRxArbiter_ResetStack_Output_marshall_struct_t;


typedef struct {
    UInt8 channel;
    gpRxArbiter_StackId_t stackId;
} gpRxArbiter_SetStackChannel_Input_struct_t;

typedef struct {
    gpRxArbiter_SetStackChannel_Input_struct_t data;
} gpRxArbiter_SetStackChannel_Input_marshall_struct_t;

typedef struct {
    gpRxArbiter_Result_t result;
} gpRxArbiter_SetStackChannel_Output_struct_t;

typedef struct {
    gpRxArbiter_SetStackChannel_Output_struct_t data;
} gpRxArbiter_SetStackChannel_Output_marshall_struct_t;


typedef struct {
    gpRxArbiter_StackId_t stackId;
} gpRxArbiter_GetStackChannel_Input_struct_t;

typedef struct {
    gpRxArbiter_GetStackChannel_Input_struct_t data;
} gpRxArbiter_GetStackChannel_Input_marshall_struct_t;

typedef struct {
    UInt8 channel;
} gpRxArbiter_GetStackChannel_Output_struct_t;

typedef struct {
    gpRxArbiter_GetStackChannel_Output_struct_t data;
} gpRxArbiter_GetStackChannel_Output_marshall_struct_t;


typedef struct {
    Bool rxon;
} gpRxArbiter_GetCurrentRxOnState_Output_struct_t;

typedef struct {
    gpRxArbiter_GetCurrentRxOnState_Output_struct_t data;
} gpRxArbiter_GetCurrentRxOnState_Output_marshall_struct_t;


typedef struct {
    UInt8 currentChannel;
} gpRxArbiter_GetCurrentRxChannel_Output_struct_t;

typedef struct {
    gpRxArbiter_GetCurrentRxChannel_Output_struct_t data;
} gpRxArbiter_GetCurrentRxChannel_Output_marshall_struct_t;


typedef struct {
    Bool enable;
    gpRxArbiter_StackId_t stackId;
} gpRxArbiter_SetStackRxOn_Input_struct_t;

typedef struct {
    gpRxArbiter_SetStackRxOn_Input_struct_t data;
} gpRxArbiter_SetStackRxOn_Input_marshall_struct_t;

typedef struct {
    gpRxArbiter_Result_t result;
} gpRxArbiter_SetStackRxOn_Output_struct_t;

typedef struct {
    gpRxArbiter_SetStackRxOn_Output_struct_t data;
} gpRxArbiter_SetStackRxOn_Output_marshall_struct_t;


typedef struct {
    gpRxArbiter_StackId_t stackId;
} gpRxArbiter_GetStackRxOn_Input_struct_t;

typedef struct {
    gpRxArbiter_GetStackRxOn_Input_struct_t data;
} gpRxArbiter_GetStackRxOn_Input_marshall_struct_t;

typedef struct {
    Bool enable;
} gpRxArbiter_GetStackRxOn_Output_struct_t;

typedef struct {
    gpRxArbiter_GetStackRxOn_Output_struct_t data;
} gpRxArbiter_GetStackRxOn_Output_marshall_struct_t;




typedef struct {
    gpRxArbiter_StackId_t stackId;
} gpRxArbiter_GetDutyCycleEnabled_Input_struct_t;

typedef struct {
    gpRxArbiter_GetDutyCycleEnabled_Input_struct_t data;
} gpRxArbiter_GetDutyCycleEnabled_Input_marshall_struct_t;

typedef struct {
    Bool result;
} gpRxArbiter_GetDutyCycleEnabled_Output_struct_t;

typedef struct {
    gpRxArbiter_GetDutyCycleEnabled_Output_struct_t data;
} gpRxArbiter_GetDutyCycleEnabled_Output_marshall_struct_t;


#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
typedef struct {
    UInt8 priority;
    gpRxArbiter_StackId_t stackId;
} gpRxArbiter_SetStackPriority_Input_struct_t;

typedef struct {
    gpRxArbiter_SetStackPriority_Input_struct_t data;
} gpRxArbiter_SetStackPriority_Input_marshall_struct_t;

typedef struct {
    gpRxArbiter_Result_t result;
} gpRxArbiter_SetStackPriority_Output_struct_t;

typedef struct {
    gpRxArbiter_SetStackPriority_Output_struct_t data;
} gpRxArbiter_SetStackPriority_Output_marshall_struct_t;

#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */

#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
typedef struct {
    gpRxArbiter_StackId_t stackId;
    gpRxArbiter_cbSetFaMode_t callback;
} gpRxArbiter_RegisterSetFaModeCallback_Input_struct_t;

typedef struct {
    gpRxArbiter_RegisterSetFaModeCallback_Input_struct_t data;
} gpRxArbiter_RegisterSetFaModeCallback_Input_marshall_struct_t;

typedef struct {
    gpRxArbiter_Result_t result;
} gpRxArbiter_RegisterSetFaModeCallback_Output_struct_t;

typedef struct {
    gpRxArbiter_RegisterSetFaModeCallback_Output_struct_t data;
} gpRxArbiter_RegisterSetFaModeCallback_Output_marshall_struct_t;

#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */

#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
typedef struct {
    gpRxArbiter_StackId_t stackId;
    gpRxArbiter_cbChannelUpdate_t callback;
} gpRxArbiter_RegisterChannelUpdateCallback_Input_struct_t;

typedef struct {
    gpRxArbiter_RegisterChannelUpdateCallback_Input_struct_t data;
} gpRxArbiter_RegisterChannelUpdateCallback_Input_marshall_struct_t;

typedef struct {
    gpRxArbiter_Result_t result;
} gpRxArbiter_RegisterChannelUpdateCallback_Output_struct_t;

typedef struct {
    gpRxArbiter_RegisterChannelUpdateCallback_Output_struct_t data;
} gpRxArbiter_RegisterChannelUpdateCallback_Output_marshall_struct_t;

#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */

typedef struct {
    gpRxArbiter_StackId_t stackId;
    UInt8 channel;
} gpRxArbiter_IsAnActiveChannel_Input_struct_t;

typedef struct {
    gpRxArbiter_IsAnActiveChannel_Input_struct_t data;
} gpRxArbiter_IsAnActiveChannel_Input_marshall_struct_t;

typedef struct {
    Bool result;
} gpRxArbiter_IsAnActiveChannel_Output_struct_t;

typedef struct {
    gpRxArbiter_IsAnActiveChannel_Output_struct_t data;
} gpRxArbiter_IsAnActiveChannel_Output_marshall_struct_t;




typedef struct {
    gpRxArbiter_StackId_t stackId;
    gpRxArbiter_FaMode_t mode;
} gpRxArbiter_cbSetFaModeNotification_Input_struct_t;

typedef struct {
    gpRxArbiter_cbSetFaModeNotification_Input_struct_t data;
} gpRxArbiter_cbSetFaModeNotification_Input_marshall_struct_t;


typedef struct {
    gpRxArbiter_StackId_t stackId;
    UInt8 channel;
} gpRxArbiter_cbChannelUpdateNotification_Input_struct_t;

typedef struct {
    gpRxArbiter_cbChannelUpdateNotification_Input_struct_t data;
} gpRxArbiter_cbChannelUpdateNotification_Input_marshall_struct_t;


typedef union {
    gpRxArbiter_ResetStack_Input_marshall_struct_t gpRxArbiter_ResetStack;
    gpRxArbiter_SetStackChannel_Input_marshall_struct_t gpRxArbiter_SetStackChannel;
    gpRxArbiter_GetStackChannel_Input_marshall_struct_t gpRxArbiter_GetStackChannel;
    gpRxArbiter_SetStackRxOn_Input_marshall_struct_t gpRxArbiter_SetStackRxOn;
    gpRxArbiter_GetStackRxOn_Input_marshall_struct_t gpRxArbiter_GetStackRxOn;
    gpRxArbiter_GetDutyCycleEnabled_Input_marshall_struct_t gpRxArbiter_GetDutyCycleEnabled;
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
    gpRxArbiter_SetStackPriority_Input_marshall_struct_t gpRxArbiter_SetStackPriority;
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
    gpRxArbiter_RegisterSetFaModeCallback_Input_marshall_struct_t gpRxArbiter_RegisterSetFaModeCallback;
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
    gpRxArbiter_RegisterChannelUpdateCallback_Input_marshall_struct_t gpRxArbiter_RegisterChannelUpdateCallback;
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
    gpRxArbiter_IsAnActiveChannel_Input_marshall_struct_t gpRxArbiter_IsAnActiveChannel;
    UInt8 dummy; //ensure none empty union definition
} gpRxArbiter_Server_Input_union_t;

typedef union {
    gpRxArbiter_ResetStack_Output_marshall_struct_t gpRxArbiter_ResetStack;
    gpRxArbiter_SetStackChannel_Output_marshall_struct_t gpRxArbiter_SetStackChannel;
    gpRxArbiter_GetStackChannel_Output_marshall_struct_t gpRxArbiter_GetStackChannel;
    gpRxArbiter_GetCurrentRxOnState_Output_marshall_struct_t gpRxArbiter_GetCurrentRxOnState;
    gpRxArbiter_GetCurrentRxChannel_Output_marshall_struct_t gpRxArbiter_GetCurrentRxChannel;
    gpRxArbiter_SetStackRxOn_Output_marshall_struct_t gpRxArbiter_SetStackRxOn;
    gpRxArbiter_GetStackRxOn_Output_marshall_struct_t gpRxArbiter_GetStackRxOn;
    gpRxArbiter_GetDutyCycleEnabled_Output_marshall_struct_t gpRxArbiter_GetDutyCycleEnabled;
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
    gpRxArbiter_SetStackPriority_Output_marshall_struct_t gpRxArbiter_SetStackPriority;
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
    gpRxArbiter_RegisterSetFaModeCallback_Output_marshall_struct_t gpRxArbiter_RegisterSetFaModeCallback;
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
    gpRxArbiter_RegisterChannelUpdateCallback_Output_marshall_struct_t gpRxArbiter_RegisterChannelUpdateCallback;
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
    gpRxArbiter_IsAnActiveChannel_Output_marshall_struct_t gpRxArbiter_IsAnActiveChannel;
    UInt8 dummy; //ensure none empty union definition
} gpRxArbiter_Server_Output_union_t;

typedef union {
    gpRxArbiter_cbSetFaModeNotification_Input_marshall_struct_t gpRxArbiter_cbSetFaModeNotification;
    gpRxArbiter_cbChannelUpdateNotification_Input_marshall_struct_t gpRxArbiter_cbChannelUpdateNotification;
    UInt8 dummy; //ensure none empty union definition
} gpRxArbiter_Client_Input_union_t;

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpRxArbiter_StackId_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpRxArbiter_StackId_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpRxArbiter_StackId_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpRxArbiter_StackId_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpRxArbiter_Result_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpRxArbiter_Result_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpRxArbiter_Result_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpRxArbiter_Result_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpRxArbiter_FaMode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpRxArbiter_FaMode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpRxArbiter_FaMode_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpRxArbiter_FaMode_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpRxArbiter_cbChannelUpdate_t_buf2api(pDest, pSource, length, pIndex) void_buf2api(pDest, pSource, length, pIndex)
#define gpRxArbiter_cbChannelUpdate_t_api2buf(pDest, pSource, length, pIndex) void_api2buf(pDest, pSource, length, pIndex)
#define gpRxArbiter_cbChannelUpdate_t_buf2api_1(pDest, pSource, pIndex)       void_buf2api_1(pDest, pSource, pIndex)
#define gpRxArbiter_cbChannelUpdate_t_api2buf_1(pDest, pSource, pIndex)       void_api2buf_1(pDest, pSource, pIndex)
#define gpRxArbiter_cbSetFaMode_t_buf2api(pDest, pSource, length, pIndex) void_buf2api(pDest, pSource, length, pIndex)
#define gpRxArbiter_cbSetFaMode_t_api2buf(pDest, pSource, length, pIndex) void_api2buf(pDest, pSource, length, pIndex)
#define gpRxArbiter_cbSetFaMode_t_buf2api_1(pDest, pSource, pIndex)       void_buf2api_1(pDest, pSource, pIndex)
#define gpRxArbiter_cbSetFaMode_t_api2buf_1(pDest, pSource, pIndex)       void_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
// Server functions
gpMarshall_AckStatus_t gpRxArbiter_ResetStack_Input_buf2api(gpRxArbiter_ResetStack_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_ResetStack_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_ResetStack_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_ResetStack_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpRxArbiter_SetStackChannel_Input_buf2api(gpRxArbiter_SetStackChannel_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_SetStackChannel_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_SetStackChannel_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_SetStackChannel_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpRxArbiter_GetStackChannel_Input_buf2api(gpRxArbiter_GetStackChannel_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_GetStackChannel_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_GetStackChannel_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_GetStackChannel_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpRxArbiter_GetCurrentRxOnState_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_GetCurrentRxOnState_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpRxArbiter_GetCurrentRxChannel_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_GetCurrentRxChannel_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpRxArbiter_SetStackRxOn_Input_buf2api(gpRxArbiter_SetStackRxOn_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_SetStackRxOn_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_SetStackRxOn_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_SetStackRxOn_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpRxArbiter_GetStackRxOn_Input_buf2api(gpRxArbiter_GetStackRxOn_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_GetStackRxOn_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_GetStackRxOn_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_GetStackRxOn_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpRxArbiter_GetDutyCycleEnabled_Input_buf2api(gpRxArbiter_GetDutyCycleEnabled_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_GetDutyCycleEnabled_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_GetDutyCycleEnabled_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_GetDutyCycleEnabled_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
gpMarshall_AckStatus_t gpRxArbiter_SetStackPriority_Input_buf2api(gpRxArbiter_SetStackPriority_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_SetStackPriority_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_SetStackPriority_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_SetStackPriority_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
gpMarshall_AckStatus_t gpRxArbiter_RegisterSetFaModeCallback_Input_buf2api(gpRxArbiter_RegisterSetFaModeCallback_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_RegisterSetFaModeCallback_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_RegisterSetFaModeCallback_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_RegisterSetFaModeCallback_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
gpMarshall_AckStatus_t gpRxArbiter_RegisterChannelUpdateCallback_Input_buf2api(gpRxArbiter_RegisterChannelUpdateCallback_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_RegisterChannelUpdateCallback_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_RegisterChannelUpdateCallback_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_RegisterChannelUpdateCallback_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
gpMarshall_AckStatus_t gpRxArbiter_IsAnActiveChannel_Input_buf2api(gpRxArbiter_IsAnActiveChannel_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_IsAnActiveChannel_Output_api2buf(UInt8Buffer* pDest , gpRxArbiter_IsAnActiveChannel_Output_marshall_struct_t* pSourceoutput , gpRxArbiter_IsAnActiveChannel_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpRxArbiter_cbSetFaModeNotification_Input_par2api(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , gpRxArbiter_FaMode_t mode , UInt16* pIndex);
void gpRxArbiter_cbChannelUpdateNotification_Input_par2api(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , UInt8 channel , UInt16* pIndex);

// Client functions
void gpRxArbiter_ResetStack_Input_par2buf(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , UInt16* pIndex);
void gpRxArbiter_ResetStack_Output_buf2par(gpRxArbiter_Result_t* result , gpRxArbiter_StackId_t stackId , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_SetStackChannel_Input_par2buf(UInt8Buffer* pDest , UInt8 channel , gpRxArbiter_StackId_t stackId , UInt16* pIndex);
void gpRxArbiter_SetStackChannel_Output_buf2par(gpRxArbiter_Result_t* result , UInt8 channel , gpRxArbiter_StackId_t stackId , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_GetStackChannel_Input_par2buf(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , UInt16* pIndex);
void gpRxArbiter_GetStackChannel_Output_buf2par(UInt8* channel , gpRxArbiter_StackId_t stackId , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_GetCurrentRxOnState_Output_buf2par(Bool* rxon , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_GetCurrentRxChannel_Output_buf2par(UInt8* currentChannel , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_SetStackRxOn_Input_par2buf(UInt8Buffer* pDest , Bool enable , gpRxArbiter_StackId_t stackId , UInt16* pIndex);
void gpRxArbiter_SetStackRxOn_Output_buf2par(gpRxArbiter_Result_t* result , Bool enable , gpRxArbiter_StackId_t stackId , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_GetStackRxOn_Input_par2buf(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , UInt16* pIndex);
void gpRxArbiter_GetStackRxOn_Output_buf2par(Bool* enable , gpRxArbiter_StackId_t stackId , UInt8Buffer* pSource , UInt16* pIndex);
void gpRxArbiter_GetDutyCycleEnabled_Input_par2buf(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , UInt16* pIndex);
void gpRxArbiter_GetDutyCycleEnabled_Output_buf2par(Bool* result , gpRxArbiter_StackId_t stackId , UInt8Buffer* pSource , UInt16* pIndex);
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
void gpRxArbiter_SetStackPriority_Input_par2buf(UInt8Buffer* pDest , UInt8 priority , gpRxArbiter_StackId_t stackId , UInt16* pIndex);
void gpRxArbiter_SetStackPriority_Output_buf2par(gpRxArbiter_Result_t* result , UInt8 priority , gpRxArbiter_StackId_t stackId , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
void gpRxArbiter_RegisterSetFaModeCallback_Input_par2buf(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , gpRxArbiter_cbSetFaMode_t callback , UInt16* pIndex);
void gpRxArbiter_RegisterSetFaModeCallback_Output_buf2par(gpRxArbiter_Result_t* result , gpRxArbiter_StackId_t stackId , gpRxArbiter_cbSetFaMode_t callback , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
#if (GP_RX_ARBITER_NUMBER_OF_STACKS > 1)
void gpRxArbiter_RegisterChannelUpdateCallback_Input_par2buf(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , gpRxArbiter_cbChannelUpdate_t callback , UInt16* pIndex);
void gpRxArbiter_RegisterChannelUpdateCallback_Output_buf2par(gpRxArbiter_Result_t* result , gpRxArbiter_StackId_t stackId , gpRxArbiter_cbChannelUpdate_t callback , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* (GP_RX_ARBITER_NUMBER_OF_STACKS > 1) */
void gpRxArbiter_IsAnActiveChannel_Input_par2buf(UInt8Buffer* pDest , gpRxArbiter_StackId_t stackId , UInt8 channel , UInt16* pIndex);
void gpRxArbiter_IsAnActiveChannel_Output_buf2par(Bool* result , gpRxArbiter_StackId_t stackId , UInt8 channel , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpRxArbiter_cbSetFaModeNotification_Input_buf2api(gpRxArbiter_cbSetFaModeNotification_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpRxArbiter_cbChannelUpdateNotification_Input_buf2api(gpRxArbiter_cbChannelUpdateNotification_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);

void gpRxArbiter_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPRXARBITER_MARSHALLING_H_


