/*
 * Copyright (c) 2016, GreenPeak Technologies
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

#ifndef _GPPD_MARSHALLING_H_
#define _GPPD_MARSHALLING_H_

//DOCUMENTATION PD: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpPd.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    gpPd_Loh_t data[1];
} gpPd_Loh_t_l1_pointer_marshall_t;



typedef struct {
    gpPd_Handle_t pdHandle;
    gpPd_Offset_t offset;
} gpPd_ReadByte_Input_struct_t;

typedef struct {
    gpPd_ReadByte_Input_struct_t data;
} gpPd_ReadByte_Input_marshall_struct_t;

typedef struct {
    UInt8 returnVal;
} gpPd_ReadByte_Output_struct_t;

typedef struct {
    gpPd_ReadByte_Output_struct_t data;
} gpPd_ReadByte_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    gpPd_Offset_t offset;
    UInt8 byte;
} gpPd_WriteByte_Input_struct_t;

typedef struct {
    gpPd_WriteByte_Input_struct_t data;
} gpPd_WriteByte_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    gpPd_Offset_t offset;
    UInt8 length;
} gpPd_ReadByteStream_Input_struct_t;

typedef struct {
    gpPd_ReadByteStream_Input_struct_t data;
} gpPd_ReadByteStream_Input_marshall_struct_t;

typedef struct {
    UInt8* pBuf;
} gpPd_ReadByteStream_Output_struct_t;

typedef struct {
    gpPd_ReadByteStream_Output_struct_t data;
    UInt8 pBuf[255];
} gpPd_ReadByteStream_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    gpPd_Offset_t offset;
    UInt8 length;
    UInt8* pBuf;
} gpPd_WriteByteStream_Input_struct_t;

typedef struct {
    gpPd_WriteByteStream_Input_struct_t data;
    UInt8 pBuf[255];
} gpPd_WriteByteStream_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t returnVal;
} gpPd_GetPd_Output_struct_t;

typedef struct {
    gpPd_GetPd_Output_struct_t data;
} gpPd_GetPd_Output_marshall_struct_t;


typedef struct {
    gpPd_BufferType_t type;
    UInt16 size;
} gpPd_GetCustomPd_Input_struct_t;

typedef struct {
    gpPd_GetCustomPd_Input_struct_t data;
} gpPd_GetCustomPd_Input_marshall_struct_t;

typedef struct {
    gpPd_Handle_t returnVal;
} gpPd_GetCustomPd_Output_struct_t;

typedef struct {
    gpPd_GetCustomPd_Output_struct_t data;
} gpPd_GetCustomPd_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_FreePd_Input_struct_t;

typedef struct {
    gpPd_FreePd_Input_struct_t data;
} gpPd_FreePd_Input_marshall_struct_t;


typedef struct {
    UInt8 pbmHandle;
    UInt16 pbmOffset;
    UInt16 pbmLength;
} gpPd_cbDataConfirm_Input_struct_t;

typedef struct {
    gpPd_cbDataConfirm_Input_struct_t data;
} gpPd_cbDataConfirm_Input_marshall_struct_t;

typedef struct {
    gpPd_Loh_t* p_PdLoh;
} gpPd_cbDataConfirm_Output_struct_t;

typedef struct {
    gpPd_cbDataConfirm_Output_struct_t data;
    gpPd_Loh_t_l1_pointer_marshall_t p_PdLoh;
} gpPd_cbDataConfirm_Output_marshall_struct_t;


typedef struct {
    UInt8 pbmHandle;
    UInt16 pbmOffset;
    UInt16 pbmLength;
    gpPd_BufferType_t type;
} gpPd_DataIndication_Input_struct_t;

typedef struct {
    gpPd_DataIndication_Input_struct_t data;
} gpPd_DataIndication_Input_marshall_struct_t;

typedef struct {
    gpPd_Loh_t* p_PdLoh;
} gpPd_DataIndication_Output_struct_t;

typedef struct {
    gpPd_DataIndication_Output_struct_t data;
    gpPd_Loh_t_l1_pointer_marshall_t p_PdLoh;
} gpPd_DataIndication_Output_marshall_struct_t;


typedef struct {
    gpPd_Loh_t* p_PdLoh;
} gpPd_DataRequest_Input_struct_t;

typedef struct {
    gpPd_DataRequest_Input_struct_t data;
    gpPd_Loh_t_l1_pointer_marshall_t p_PdLoh;
} gpPd_DataRequest_Input_marshall_struct_t;

typedef struct {
    UInt8 returnVal;
} gpPd_DataRequest_Output_struct_t;

typedef struct {
    gpPd_DataRequest_Output_struct_t data;
} gpPd_DataRequest_Output_marshall_struct_t;


typedef struct {
    UInt8 pbmHandle;
} gpPd_cbPurgeConfirm_Input_struct_t;

typedef struct {
    gpPd_cbPurgeConfirm_Input_struct_t data;
} gpPd_cbPurgeConfirm_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_PurgeRequest_Input_struct_t;

typedef struct {
    gpPd_PurgeRequest_Input_struct_t data;
} gpPd_PurgeRequest_Input_marshall_struct_t;

typedef struct {
    gpPd_Handle_t returnVal;
} gpPd_PurgeRequest_Output_struct_t;

typedef struct {
    gpPd_PurgeRequest_Output_struct_t data;
} gpPd_PurgeRequest_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    UInt8 dataOffset;
    UInt8 dataLength;
    UInt8 auxOffset;
    UInt8 auxLength;
} gpPd_SecRequest_Input_struct_t;

typedef struct {
    gpPd_SecRequest_Input_struct_t data;
} gpPd_SecRequest_Input_marshall_struct_t;

typedef struct {
    UInt8 returnVal;
} gpPd_SecRequest_Output_struct_t;

typedef struct {
    gpPd_SecRequest_Output_struct_t data;
} gpPd_SecRequest_Output_marshall_struct_t;


typedef struct {
    UInt8 pbmHandle;
    UInt8 dataOffset;
    UInt8 dataLength;
} gpPd_cbSecConfirm_Input_struct_t;

typedef struct {
    gpPd_cbSecConfirm_Input_struct_t data;
} gpPd_cbSecConfirm_Input_marshall_struct_t;

typedef struct {
    gpPd_Handle_t returnVal;
} gpPd_cbSecConfirm_Output_struct_t;

typedef struct {
    gpPd_cbSecConfirm_Output_struct_t data;
} gpPd_cbSecConfirm_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_CheckPdValid_Input_struct_t;

typedef struct {
    gpPd_CheckPdValid_Input_struct_t data;
} gpPd_CheckPdValid_Input_marshall_struct_t;

typedef struct {
    gpPd_Result_t returnVal;
} gpPd_CheckPdValid_Output_struct_t;

typedef struct {
    gpPd_CheckPdValid_Output_struct_t data;
} gpPd_CheckPdValid_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetRssi_Input_struct_t;

typedef struct {
    gpPd_GetRssi_Input_struct_t data;
} gpPd_GetRssi_Input_marshall_struct_t;

typedef struct {
    gpPd_Rssi_t rssi;
} gpPd_GetRssi_Output_struct_t;

typedef struct {
    gpPd_GetRssi_Output_struct_t data;
} gpPd_GetRssi_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    gpPd_Rssi_t rssi;
} gpPd_SetRssi_Input_struct_t;

typedef struct {
    gpPd_SetRssi_Input_struct_t data;
} gpPd_SetRssi_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetLqi_Input_struct_t;

typedef struct {
    gpPd_GetLqi_Input_struct_t data;
} gpPd_GetLqi_Input_marshall_struct_t;

typedef struct {
    gpPd_Lqi_t lqi;
} gpPd_GetLqi_Output_struct_t;

typedef struct {
    gpPd_GetLqi_Output_struct_t data;
} gpPd_GetLqi_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    gpPd_Lqi_t lqi;
} gpPd_SetLqi_Input_struct_t;

typedef struct {
    gpPd_SetLqi_Input_struct_t data;
} gpPd_SetLqi_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetRxTimestamp_Input_struct_t;

typedef struct {
    gpPd_GetRxTimestamp_Input_struct_t data;
} gpPd_GetRxTimestamp_Input_marshall_struct_t;

typedef struct {
    UInt32 timestamp;
} gpPd_GetRxTimestamp_Output_struct_t;

typedef struct {
    gpPd_GetRxTimestamp_Output_struct_t data;
} gpPd_GetRxTimestamp_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetTxTimestamp_Input_struct_t;

typedef struct {
    gpPd_GetTxTimestamp_Input_struct_t data;
} gpPd_GetTxTimestamp_Input_marshall_struct_t;

typedef struct {
    UInt32 timestamp;
} gpPd_GetTxTimestamp_Output_struct_t;

typedef struct {
    gpPd_GetTxTimestamp_Output_struct_t data;
} gpPd_GetTxTimestamp_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    gpPd_TimeStamp_t timestamp;
} gpPd_SetRxTimestamp_Input_struct_t;

typedef struct {
    gpPd_SetRxTimestamp_Input_struct_t data;
} gpPd_SetRxTimestamp_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    gpPd_TimeStamp_t timestamp;
} gpPd_SetTxTimestamp_Input_struct_t;

typedef struct {
    gpPd_SetTxTimestamp_Input_struct_t data;
} gpPd_SetTxTimestamp_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_CopyPd_Input_struct_t;

typedef struct {
    gpPd_CopyPd_Input_struct_t data;
} gpPd_CopyPd_Input_marshall_struct_t;

typedef struct {
    gpPd_Handle_t copyPd;
} gpPd_CopyPd_Output_struct_t;

typedef struct {
    gpPd_CopyPd_Output_struct_t data;
} gpPd_CopyPd_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetTxRetryCntr_Input_struct_t;

typedef struct {
    gpPd_GetTxRetryCntr_Input_struct_t data;
} gpPd_GetTxRetryCntr_Input_marshall_struct_t;

typedef struct {
    UInt8 retryCntr;
} gpPd_GetTxRetryCntr_Output_struct_t;

typedef struct {
    gpPd_GetTxRetryCntr_Output_struct_t data;
} gpPd_GetTxRetryCntr_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetRxTimestampChip_Input_struct_t;

typedef struct {
    gpPd_GetRxTimestampChip_Input_struct_t data;
} gpPd_GetRxTimestampChip_Input_marshall_struct_t;

typedef struct {
    gpPd_TimeStamp_t timestamp;
} gpPd_GetRxTimestampChip_Output_struct_t;

typedef struct {
    gpPd_GetRxTimestampChip_Output_struct_t data;
} gpPd_GetRxTimestampChip_Output_marshall_struct_t;


typedef struct {
    gpPd_Loh_t* pPdLoh;
    UInt8 length;
    UInt8* pData;  //FIXME - Manual
} gpPd_AppendWithUpdate_Input_struct_t;

typedef struct {
    gpPd_AppendWithUpdate_Input_struct_t data;
    gpPd_Loh_t_l1_pointer_marshall_t pPdLoh;
    UInt8 pData[128];
} gpPd_AppendWithUpdate_Input_marshall_struct_t;

typedef struct {
    // pPdLoh used from input structure because it is an inout parameter
    char _unused_dummy; // struct shall not be empty in C99
} gpPd_AppendWithUpdate_Output_struct_t;

typedef struct {
    gpPd_AppendWithUpdate_Output_struct_t data;
    // pPdLoh used from input structure because it is an inout parameter
} gpPd_AppendWithUpdate_Output_marshall_struct_t;


typedef struct {
    gpPd_Loh_t* pPdLoh;
    UInt8 length;
    UInt8* pData;
} gpPd_PrependWithUpdate_Input_struct_t;

typedef struct {
    gpPd_PrependWithUpdate_Input_struct_t data;
    gpPd_Loh_t_l1_pointer_marshall_t pPdLoh;
    UInt8 pData[128];
} gpPd_PrependWithUpdate_Input_marshall_struct_t;

typedef struct {
    // pPdLoh used from input structure because it is an inout parameter
    char _unused_dummy; // struct shall not be empty in C99
} gpPd_PrependWithUpdate_Output_struct_t;

typedef struct {
    gpPd_PrependWithUpdate_Output_struct_t data;
    // pPdLoh used from input structure because it is an inout parameter
} gpPd_PrependWithUpdate_Output_marshall_struct_t;


typedef struct {
    gpPd_Loh_t* pPdLoh;
    UInt8 length;
} gpPd_ReadWithUpdate_Input_struct_t;

typedef struct {
    gpPd_ReadWithUpdate_Input_struct_t data;
    gpPd_Loh_t_l1_pointer_marshall_t pPdLoh;
} gpPd_ReadWithUpdate_Input_marshall_struct_t;

typedef struct {
    // pPdLoh used from input structure because it is an inout parameter
    UInt8* pData;
} gpPd_ReadWithUpdate_Output_struct_t;

typedef struct {
    gpPd_ReadWithUpdate_Output_struct_t data;
    // pPdLoh used from input structure because it is an inout parameter
    UInt8 pData[128];
} gpPd_ReadWithUpdate_Output_marshall_struct_t;

typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetRxTimestampExt_Input_struct_t;  //FIXME - Manual

typedef struct {
    gpPd_GetRxTimestampExt_Input_struct_t data;
} gpPd_GetRxTimestampExt_Input_marshall_struct_t;  //FIXME - Manual

typedef struct {
    gpPd_TimeStamp_t timestamp;
} gpPd_GetRxTimestampExt_Output_struct_t;  //FIXME - Manual

typedef struct {
    gpPd_GetRxTimestampExt_Output_struct_t data;
} gpPd_GetRxTimestampExt_Output_marshall_struct_t;  //FIXME - Manual


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetTxTimestampExt_Input_struct_t;  //FIXME - Manual

typedef struct {
    gpPd_GetTxTimestampExt_Input_struct_t data;
} gpPd_GetTxTimestampExt_Input_marshall_struct_t;  //FIXME - Manual

typedef struct {
    gpPd_TimeStamp_t timestamp;
} gpPd_GetTxTimestampExt_Output_struct_t;  //FIXME - Manual

typedef struct {
    gpPd_GetTxTimestampExt_Output_struct_t data;
} gpPd_GetTxTimestampExt_Output_marshall_struct_t;  //FIXME - Manual


typedef struct {
    gpPd_Handle_t pdHandle;
    UInt8 retryCntr;
} gpPd_SetTxRetryCntr_Input_struct_t;

typedef struct {
    gpPd_SetTxRetryCntr_Input_struct_t data;
} gpPd_SetTxRetryCntr_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetRxChannel_Input_struct_t;

typedef struct {
    gpPd_GetRxChannel_Input_struct_t data;
} gpPd_GetRxChannel_Input_marshall_struct_t;

typedef struct {
    UInt8 rxChannel;
} gpPd_GetRxChannel_Output_struct_t;

typedef struct {
    gpPd_GetRxChannel_Output_struct_t data;
} gpPd_GetRxChannel_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    UInt8 rxChannel;
} gpPd_SetRxChannel_Input_struct_t;

typedef struct {
    gpPd_SetRxChannel_Input_struct_t data;
} gpPd_SetRxChannel_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetFramePendingAfterTx_Input_struct_t;

typedef struct {
    gpPd_GetFramePendingAfterTx_Input_struct_t data;
} gpPd_GetFramePendingAfterTx_Input_marshall_struct_t;

typedef struct {
    UInt8 framePending;
} gpPd_GetFramePendingAfterTx_Output_struct_t;

typedef struct {
    gpPd_GetFramePendingAfterTx_Output_struct_t data;
} gpPd_GetFramePendingAfterTx_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    UInt8 framePending;
} gpPd_SetFramePendingAfterTx_Input_struct_t;

typedef struct {
    gpPd_SetFramePendingAfterTx_Input_struct_t data;
} gpPd_SetFramePendingAfterTx_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetTxCCACntr_Input_struct_t;

typedef struct {
    gpPd_GetTxCCACntr_Input_struct_t data;
} gpPd_GetTxCCACntr_Input_marshall_struct_t;

typedef struct {
    UInt8 txCCACntr;
} gpPd_GetTxCCACntr_Output_struct_t;

typedef struct {
    gpPd_GetTxCCACntr_Output_struct_t data;
} gpPd_GetTxCCACntr_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetTxAckLqi_Input_struct_t;

typedef struct {
    gpPd_GetTxAckLqi_Input_struct_t data;
} gpPd_GetTxAckLqi_Input_marshall_struct_t;

typedef struct {
    gpPd_Lqi_t ackLqi;
} gpPd_GetTxAckLqi_Output_struct_t;

typedef struct {
    gpPd_GetTxAckLqi_Output_struct_t data;
} gpPd_GetTxAckLqi_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetFrameControlFromTxAckAfterRx_Input_struct_t;

typedef struct {
    gpPd_GetFrameControlFromTxAckAfterRx_Input_struct_t data;
} gpPd_GetFrameControlFromTxAckAfterRx_Input_marshall_struct_t;

typedef struct {
    UInt16 framePending;
} gpPd_GetFrameControlFromTxAckAfterRx_Output_struct_t;

typedef struct {
    gpPd_GetFrameControlFromTxAckAfterRx_Output_struct_t data;
} gpPd_GetFrameControlFromTxAckAfterRx_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    UInt16 framePending;
} gpPd_SetFrameControlFromTxAckAfterRx_Input_struct_t;

typedef struct {
    gpPd_SetFrameControlFromTxAckAfterRx_Input_struct_t data;
} gpPd_SetFrameControlFromTxAckAfterRx_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetRxEnhancedAckFromTxPbm_Input_struct_t;

typedef struct {
    gpPd_GetRxEnhancedAckFromTxPbm_Input_struct_t data;
} gpPd_GetRxEnhancedAckFromTxPbm_Input_marshall_struct_t;

typedef struct {
    Bool enhancedAck;
} gpPd_GetRxEnhancedAckFromTxPbm_Output_struct_t;

typedef struct {
    gpPd_GetRxEnhancedAckFromTxPbm_Output_struct_t data;
} gpPd_GetRxEnhancedAckFromTxPbm_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    Bool enhancedAck;
} gpPd_SetRxEnhancedAckFromTxPbm_Input_struct_t;

typedef struct {
    gpPd_SetRxEnhancedAckFromTxPbm_Input_struct_t data;
} gpPd_SetRxEnhancedAckFromTxPbm_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetFrameCounterFromTxAckAfterRx_Input_struct_t;

typedef struct {
    gpPd_GetFrameCounterFromTxAckAfterRx_Input_struct_t data;
} gpPd_GetFrameCounterFromTxAckAfterRx_Input_marshall_struct_t;

typedef struct {
    UInt32 frameCounter;
} gpPd_GetFrameCounterFromTxAckAfterRx_Output_struct_t;

typedef struct {
    gpPd_GetFrameCounterFromTxAckAfterRx_Output_struct_t data;
} gpPd_GetFrameCounterFromTxAckAfterRx_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    UInt32 frameCounter;
} gpPd_SetFrameCounterFromTxAckAfterRx_Input_struct_t;

typedef struct {
    gpPd_SetFrameCounterFromTxAckAfterRx_Input_struct_t data;
} gpPd_SetFrameCounterFromTxAckAfterRx_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetKeyIdFromTxAckAfterRx_Input_struct_t;

typedef struct {
    gpPd_GetKeyIdFromTxAckAfterRx_Input_struct_t data;
} gpPd_GetKeyIdFromTxAckAfterRx_Input_marshall_struct_t;

typedef struct {
    UInt8 keyId;
} gpPd_GetKeyIdFromTxAckAfterRx_Output_struct_t;

typedef struct {
    gpPd_GetKeyIdFromTxAckAfterRx_Output_struct_t data;
} gpPd_GetKeyIdFromTxAckAfterRx_Output_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
    UInt8 keyId;
} gpPd_SetKeyIdFromTxAckAfterRx_Input_struct_t;

typedef struct {
    gpPd_SetKeyIdFromTxAckAfterRx_Input_struct_t data;
} gpPd_SetKeyIdFromTxAckAfterRx_Input_marshall_struct_t;


typedef struct {
    gpPd_Handle_t pdHandle;
} gpPd_GetTxChannel_Input_struct_t;

typedef struct {
    gpPd_GetTxChannel_Input_struct_t data;
} gpPd_GetTxChannel_Input_marshall_struct_t;

typedef struct {
    UInt8 channel;
} gpPd_GetTxChannel_Output_struct_t;

typedef struct {
    gpPd_GetTxChannel_Output_struct_t data;
} gpPd_GetTxChannel_Output_marshall_struct_t;


typedef union {
    gpPd_ReadByte_Input_marshall_struct_t gpPd_ReadByte;
    gpPd_WriteByte_Input_marshall_struct_t gpPd_WriteByte;
    gpPd_ReadByteStream_Input_marshall_struct_t gpPd_ReadByteStream;
    gpPd_WriteByteStream_Input_marshall_struct_t gpPd_WriteByteStream;
    gpPd_GetCustomPd_Input_marshall_struct_t gpPd_GetCustomPd;
    gpPd_FreePd_Input_marshall_struct_t gpPd_FreePd;
    gpPd_cbDataConfirm_Input_marshall_struct_t gpPd_cbDataConfirm;
    gpPd_DataIndication_Input_marshall_struct_t gpPd_DataIndication;
    gpPd_DataRequest_Input_marshall_struct_t gpPd_DataRequest;
    gpPd_cbPurgeConfirm_Input_marshall_struct_t gpPd_cbPurgeConfirm;
    gpPd_PurgeRequest_Input_marshall_struct_t gpPd_PurgeRequest;
    gpPd_SecRequest_Input_marshall_struct_t gpPd_SecRequest;
    gpPd_cbSecConfirm_Input_marshall_struct_t gpPd_cbSecConfirm;
    gpPd_CheckPdValid_Input_marshall_struct_t gpPd_CheckPdValid;
    gpPd_GetRssi_Input_marshall_struct_t gpPd_GetRssi;
    gpPd_SetRssi_Input_marshall_struct_t gpPd_SetRssi;
    gpPd_GetLqi_Input_marshall_struct_t gpPd_GetLqi;
    gpPd_SetLqi_Input_marshall_struct_t gpPd_SetLqi;
    gpPd_GetRxTimestamp_Input_marshall_struct_t gpPd_GetRxTimestamp;
    gpPd_GetTxTimestamp_Input_marshall_struct_t gpPd_GetTxTimestamp;
    gpPd_SetRxTimestamp_Input_marshall_struct_t gpPd_SetRxTimestamp;
    gpPd_SetTxTimestamp_Input_marshall_struct_t gpPd_SetTxTimestamp;
    gpPd_CopyPd_Input_marshall_struct_t gpPd_CopyPd;
    gpPd_GetTxRetryCntr_Input_marshall_struct_t gpPd_GetTxRetryCntr;
    gpPd_GetRxTimestampChip_Input_marshall_struct_t gpPd_GetRxTimestampChip;
    gpPd_AppendWithUpdate_Input_marshall_struct_t gpPd_AppendWithUpdate;
    gpPd_PrependWithUpdate_Input_marshall_struct_t gpPd_PrependWithUpdate;
    gpPd_ReadWithUpdate_Input_marshall_struct_t gpPd_ReadWithUpdate;
    gpPd_GetRxTimestampExt_Input_marshall_struct_t gpPd_GetRxTimestampExt;  //FIXME - Manual
    gpPd_GetTxTimestampExt_Input_marshall_struct_t gpPd_GetTxTimestampExt;  //FIXME - Manual
    gpPd_SetTxRetryCntr_Input_marshall_struct_t gpPd_SetTxRetryCntr;
    gpPd_GetRxChannel_Input_marshall_struct_t gpPd_GetRxChannel;
    gpPd_SetRxChannel_Input_marshall_struct_t gpPd_SetRxChannel;
    gpPd_GetFramePendingAfterTx_Input_marshall_struct_t gpPd_GetFramePendingAfterTx;
    gpPd_SetFramePendingAfterTx_Input_marshall_struct_t gpPd_SetFramePendingAfterTx;
    gpPd_GetTxCCACntr_Input_marshall_struct_t gpPd_GetTxCCACntr;
    gpPd_GetTxAckLqi_Input_marshall_struct_t gpPd_GetTxAckLqi;
    gpPd_GetFrameControlFromTxAckAfterRx_Input_marshall_struct_t gpPd_GetFrameControlFromTxAckAfterRx;
    gpPd_SetFrameControlFromTxAckAfterRx_Input_marshall_struct_t gpPd_SetFrameControlFromTxAckAfterRx;
    gpPd_GetRxEnhancedAckFromTxPbm_Input_marshall_struct_t gpPd_GetRxEnhancedAckFromTxPbm;
    gpPd_SetRxEnhancedAckFromTxPbm_Input_marshall_struct_t gpPd_SetRxEnhancedAckFromTxPbm;
    gpPd_GetFrameCounterFromTxAckAfterRx_Input_marshall_struct_t gpPd_GetFrameCounterFromTxAckAfterRx;
    gpPd_SetFrameCounterFromTxAckAfterRx_Input_marshall_struct_t gpPd_SetFrameCounterFromTxAckAfterRx;
    gpPd_GetKeyIdFromTxAckAfterRx_Input_marshall_struct_t gpPd_GetKeyIdFromTxAckAfterRx;
    gpPd_SetKeyIdFromTxAckAfterRx_Input_marshall_struct_t gpPd_SetKeyIdFromTxAckAfterRx;
    gpPd_GetTxChannel_Input_marshall_struct_t gpPd_GetTxChannel;
    UInt8 dummy; //ensure none empty union definition
} gpPd_Server_Input_union_t;

typedef union {
    gpPd_ReadByte_Output_marshall_struct_t gpPd_ReadByte;
    gpPd_ReadByteStream_Output_marshall_struct_t gpPd_ReadByteStream;
    gpPd_GetPd_Output_marshall_struct_t gpPd_GetPd;
    gpPd_GetCustomPd_Output_marshall_struct_t gpPd_GetCustomPd;
    gpPd_cbDataConfirm_Output_marshall_struct_t gpPd_cbDataConfirm;
    gpPd_DataIndication_Output_marshall_struct_t gpPd_DataIndication;
    gpPd_DataRequest_Output_marshall_struct_t gpPd_DataRequest;
    gpPd_PurgeRequest_Output_marshall_struct_t gpPd_PurgeRequest;
    gpPd_SecRequest_Output_marshall_struct_t gpPd_SecRequest;
    gpPd_cbSecConfirm_Output_marshall_struct_t gpPd_cbSecConfirm;
    gpPd_CheckPdValid_Output_marshall_struct_t gpPd_CheckPdValid;
    gpPd_GetRssi_Output_marshall_struct_t gpPd_GetRssi;
    gpPd_GetLqi_Output_marshall_struct_t gpPd_GetLqi;
    gpPd_GetRxTimestamp_Output_marshall_struct_t gpPd_GetRxTimestamp;
    gpPd_GetTxTimestamp_Output_marshall_struct_t gpPd_GetTxTimestamp;
    gpPd_CopyPd_Output_marshall_struct_t gpPd_CopyPd;
    gpPd_GetTxRetryCntr_Output_marshall_struct_t gpPd_GetTxRetryCntr;
    gpPd_GetRxTimestampChip_Output_marshall_struct_t gpPd_GetRxTimestampChip;
    gpPd_AppendWithUpdate_Output_marshall_struct_t gpPd_AppendWithUpdate;
    gpPd_PrependWithUpdate_Output_marshall_struct_t gpPd_PrependWithUpdate;
    gpPd_ReadWithUpdate_Output_marshall_struct_t gpPd_ReadWithUpdate;
    gpPd_GetRxTimestampExt_Output_marshall_struct_t gpPd_GetRxTimestampExt;  //FIXME - Manual
    gpPd_GetTxTimestampExt_Output_marshall_struct_t gpPd_GetTxTimestampExt;  //FIXME - Manual
    gpPd_GetRxChannel_Output_marshall_struct_t gpPd_GetRxChannel;
    gpPd_GetFramePendingAfterTx_Output_marshall_struct_t gpPd_GetFramePendingAfterTx;
    gpPd_GetTxCCACntr_Output_marshall_struct_t gpPd_GetTxCCACntr;
    gpPd_GetTxAckLqi_Output_marshall_struct_t gpPd_GetTxAckLqi;
    gpPd_GetFrameControlFromTxAckAfterRx_Output_marshall_struct_t gpPd_GetFrameControlFromTxAckAfterRx;
    gpPd_GetRxEnhancedAckFromTxPbm_Output_marshall_struct_t gpPd_GetRxEnhancedAckFromTxPbm;
    gpPd_GetFrameCounterFromTxAckAfterRx_Output_marshall_struct_t gpPd_GetFrameCounterFromTxAckAfterRx;
    gpPd_GetKeyIdFromTxAckAfterRx_Output_marshall_struct_t gpPd_GetKeyIdFromTxAckAfterRx;
    gpPd_GetTxChannel_Output_marshall_struct_t gpPd_GetTxChannel;
    UInt8 dummy; //ensure none empty union definition
} gpPd_Server_Output_union_t;
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpPd_Handle_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPd_Handle_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPd_Handle_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPd_Handle_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPd_Offset_t_buf2api(pDest, pSource, length, pIndex) UInt16_buf2api(pDest, pSource, length, pIndex)
#define gpPd_Offset_t_api2buf(pDest, pSource, length, pIndex) UInt16_api2buf(pDest, pSource, length, pIndex)
#define gpPd_Offset_t_buf2api_1(pDest, pSource, pIndex)       UInt16_buf2api_1(pDest, pSource, pIndex)
#define gpPd_Offset_t_api2buf_1(pDest, pSource, pIndex)       UInt16_api2buf_1(pDest, pSource, pIndex)
#define gpPd_Length_t_buf2api(pDest, pSource, length, pIndex) UInt16_buf2api(pDest, pSource, length, pIndex)
#define gpPd_Length_t_api2buf(pDest, pSource, length, pIndex) UInt16_api2buf(pDest, pSource, length, pIndex)
#define gpPd_Length_t_buf2api_1(pDest, pSource, pIndex)       UInt16_buf2api_1(pDest, pSource, pIndex)
#define gpPd_Length_t_api2buf_1(pDest, pSource, pIndex)       UInt16_api2buf_1(pDest, pSource, pIndex)
#define gpPd_Lqi_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPd_Lqi_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPd_Lqi_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPd_Lqi_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPd_Rssi_t_buf2api(pDest, pSource, length, pIndex) Int8_buf2api(pDest, pSource, length, pIndex)
#define gpPd_Rssi_t_api2buf(pDest, pSource, length, pIndex) Int8_api2buf(pDest, pSource, length, pIndex)
#define gpPd_Rssi_t_buf2api_1(pDest, pSource, pIndex)       Int8_buf2api_1(pDest, pSource, pIndex)
#define gpPd_Rssi_t_api2buf_1(pDest, pSource, pIndex)       Int8_api2buf_1(pDest, pSource, pIndex)
#define gpPd_Result_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPd_Result_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPd_Result_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPd_Result_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPd_BufferType_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpPd_BufferType_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpPd_BufferType_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpPd_BufferType_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpPd_TimeStamp_t_buf2api(pDest, pSource, length, pIndex) UInt32_buf2api(pDest, pSource, length, pIndex)
#define gpPd_TimeStamp_t_api2buf(pDest, pSource, length, pIndex) UInt32_api2buf(pDest, pSource, length, pIndex)
#define gpPd_TimeStamp_t_buf2api_1(pDest, pSource, pIndex)       UInt32_buf2api_1(pDest, pSource, pIndex)
#define gpPd_TimeStamp_t_api2buf_1(pDest, pSource, pIndex)       UInt32_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
gpMarshall_AckStatus_t gpPd_Loh_t_buf2api(gpPd_Loh_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex , Bool storePdHandle );
void gpPd_Loh_t_api2buf(UInt8Buffer* pDest , const gpPd_Loh_t* pSource , UInt16 length , UInt16* pIndex);
// Server functions
gpMarshall_AckStatus_t gpPd_ReadByte_Input_buf2api(gpPd_ReadByte_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_ReadByte_Output_api2buf(UInt8Buffer* pDest , gpPd_ReadByte_Output_marshall_struct_t* pSourceoutput , gpPd_ReadByte_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_WriteByte_Input_buf2api(gpPd_WriteByte_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_ReadByteStream_Input_buf2api(gpPd_ReadByteStream_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_ReadByteStream_Output_api2buf(UInt8Buffer* pDest , gpPd_ReadByteStream_Output_marshall_struct_t* pSourceoutput , gpPd_ReadByteStream_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_WriteByteStream_Input_buf2api(gpPd_WriteByteStream_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetPd_Output_api2buf(UInt8Buffer* pDest , gpPd_GetPd_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetCustomPd_Input_buf2api(gpPd_GetCustomPd_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetCustomPd_Output_api2buf(UInt8Buffer* pDest , gpPd_GetCustomPd_Output_marshall_struct_t* pSourceoutput , gpPd_GetCustomPd_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_FreePd_Input_buf2api(gpPd_FreePd_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_cbDataConfirm_Input_buf2api(gpPd_cbDataConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_cbDataConfirm_Output_api2buf(UInt8Buffer* pDest , gpPd_cbDataConfirm_Output_marshall_struct_t* pSourceoutput , gpPd_cbDataConfirm_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_DataIndication_Input_buf2api(gpPd_DataIndication_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_DataIndication_Output_api2buf(UInt8Buffer* pDest , gpPd_DataIndication_Output_marshall_struct_t* pSourceoutput , gpPd_DataIndication_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_DataRequest_Input_buf2api(gpPd_DataRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_DataRequest_Output_api2buf(UInt8Buffer* pDest , gpPd_DataRequest_Output_marshall_struct_t* pSourceoutput , gpPd_DataRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_cbPurgeConfirm_Input_buf2api(gpPd_cbPurgeConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_PurgeRequest_Input_buf2api(gpPd_PurgeRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_PurgeRequest_Output_api2buf(UInt8Buffer* pDest , gpPd_PurgeRequest_Output_marshall_struct_t* pSourceoutput , gpPd_PurgeRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SecRequest_Input_buf2api(gpPd_SecRequest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SecRequest_Output_api2buf(UInt8Buffer* pDest , gpPd_SecRequest_Output_marshall_struct_t* pSourceoutput , gpPd_SecRequest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_cbSecConfirm_Input_buf2api(gpPd_cbSecConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_cbSecConfirm_Output_api2buf(UInt8Buffer* pDest , gpPd_cbSecConfirm_Output_marshall_struct_t* pSourceoutput , gpPd_cbSecConfirm_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_CheckPdValid_Input_buf2api(gpPd_CheckPdValid_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_CheckPdValid_Output_api2buf(UInt8Buffer* pDest , gpPd_CheckPdValid_Output_marshall_struct_t* pSourceoutput , gpPd_CheckPdValid_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetRssi_Input_buf2api(gpPd_GetRssi_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetRssi_Output_api2buf(UInt8Buffer* pDest , gpPd_GetRssi_Output_marshall_struct_t* pSourceoutput , gpPd_GetRssi_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetRssi_Input_buf2api(gpPd_SetRssi_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetLqi_Input_buf2api(gpPd_GetLqi_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetLqi_Output_api2buf(UInt8Buffer* pDest , gpPd_GetLqi_Output_marshall_struct_t* pSourceoutput , gpPd_GetLqi_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetLqi_Input_buf2api(gpPd_SetLqi_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetRxTimestamp_Input_buf2api(gpPd_GetRxTimestamp_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetRxTimestamp_Output_api2buf(UInt8Buffer* pDest , gpPd_GetRxTimestamp_Output_marshall_struct_t* pSourceoutput , gpPd_GetRxTimestamp_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetTxTimestamp_Input_buf2api(gpPd_GetTxTimestamp_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetTxTimestamp_Output_api2buf(UInt8Buffer* pDest , gpPd_GetTxTimestamp_Output_marshall_struct_t* pSourceoutput , gpPd_GetTxTimestamp_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetRxTimestamp_Input_buf2api(gpPd_SetRxTimestamp_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetTxTimestamp_Input_buf2api(gpPd_SetTxTimestamp_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_CopyPd_Input_buf2api(gpPd_CopyPd_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_CopyPd_Output_api2buf(UInt8Buffer* pDest , gpPd_CopyPd_Output_marshall_struct_t* pSourceoutput , gpPd_CopyPd_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetTxRetryCntr_Input_buf2api(gpPd_GetTxRetryCntr_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetTxRetryCntr_Output_api2buf(UInt8Buffer* pDest , gpPd_GetTxRetryCntr_Output_marshall_struct_t* pSourceoutput , gpPd_GetTxRetryCntr_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetRxTimestampChip_Input_buf2api(gpPd_GetRxTimestampChip_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetRxTimestampChip_Output_api2buf(UInt8Buffer* pDest , gpPd_GetRxTimestampChip_Output_marshall_struct_t* pSourceoutput , gpPd_GetRxTimestampChip_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_AppendWithUpdate_Input_buf2api(gpPd_AppendWithUpdate_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_AppendWithUpdate_Output_api2buf(UInt8Buffer* pDest , gpPd_AppendWithUpdate_Output_marshall_struct_t* pSourceoutput , gpPd_AppendWithUpdate_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_PrependWithUpdate_Input_buf2api(gpPd_PrependWithUpdate_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_PrependWithUpdate_Output_api2buf(UInt8Buffer* pDest , gpPd_PrependWithUpdate_Output_marshall_struct_t* pSourceoutput , gpPd_PrependWithUpdate_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_ReadWithUpdate_Input_buf2api(gpPd_ReadWithUpdate_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_ReadWithUpdate_Output_api2buf(UInt8Buffer* pDest , gpPd_ReadWithUpdate_Output_marshall_struct_t* pSourceoutput , gpPd_ReadWithUpdate_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetRxTimestampExt_Input_buf2api( gpPd_GetRxTimestampExt_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );  //FIXME - Manual
void gpPd_GetRxTimestampExt_Output_api2buf( UInt8Buffer* pDest , gpPd_GetRxTimestampExt_Output_marshall_struct_t* pSourceoutput , gpPd_GetRxTimestampExt_Input_marshall_struct_t* pSourceinput , UInt16* pIndex );  //FIXME - Manual
gpMarshall_AckStatus_t gpPd_GetTxTimestampExt_Input_buf2api( gpPd_GetTxTimestampExt_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex );  //FIXME - Manual
void gpPd_GetTxTimestampExt_Output_api2buf( UInt8Buffer* pDest , gpPd_GetTxTimestampExt_Output_marshall_struct_t* pSourceoutput , gpPd_GetTxTimestampExt_Input_marshall_struct_t* pSourceinput , UInt16* pIndex );  //FIXME - Manual
gpMarshall_AckStatus_t gpPd_SetTxRetryCntr_Input_buf2api(gpPd_SetTxRetryCntr_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetRxChannel_Input_buf2api(gpPd_GetRxChannel_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetRxChannel_Output_api2buf(UInt8Buffer* pDest , gpPd_GetRxChannel_Output_marshall_struct_t* pSourceoutput , gpPd_GetRxChannel_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetRxChannel_Input_buf2api(gpPd_SetRxChannel_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetFramePendingAfterTx_Input_buf2api(gpPd_GetFramePendingAfterTx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetFramePendingAfterTx_Output_api2buf(UInt8Buffer* pDest , gpPd_GetFramePendingAfterTx_Output_marshall_struct_t* pSourceoutput , gpPd_GetFramePendingAfterTx_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetFramePendingAfterTx_Input_buf2api(gpPd_SetFramePendingAfterTx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetTxCCACntr_Input_buf2api(gpPd_GetTxCCACntr_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetTxCCACntr_Output_api2buf(UInt8Buffer* pDest , gpPd_GetTxCCACntr_Output_marshall_struct_t* pSourceoutput , gpPd_GetTxCCACntr_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetTxAckLqi_Input_buf2api(gpPd_GetTxAckLqi_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetTxAckLqi_Output_api2buf(UInt8Buffer* pDest , gpPd_GetTxAckLqi_Output_marshall_struct_t* pSourceoutput , gpPd_GetTxAckLqi_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetFrameControlFromTxAckAfterRx_Input_buf2api(gpPd_GetFrameControlFromTxAckAfterRx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetFrameControlFromTxAckAfterRx_Output_api2buf(UInt8Buffer* pDest , gpPd_GetFrameControlFromTxAckAfterRx_Output_marshall_struct_t* pSourceoutput , gpPd_GetFrameControlFromTxAckAfterRx_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetFrameControlFromTxAckAfterRx_Input_buf2api(gpPd_SetFrameControlFromTxAckAfterRx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetRxEnhancedAckFromTxPbm_Input_buf2api(gpPd_GetRxEnhancedAckFromTxPbm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetRxEnhancedAckFromTxPbm_Output_api2buf(UInt8Buffer* pDest , gpPd_GetRxEnhancedAckFromTxPbm_Output_marshall_struct_t* pSourceoutput , gpPd_GetRxEnhancedAckFromTxPbm_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetRxEnhancedAckFromTxPbm_Input_buf2api(gpPd_SetRxEnhancedAckFromTxPbm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetFrameCounterFromTxAckAfterRx_Input_buf2api(gpPd_GetFrameCounterFromTxAckAfterRx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetFrameCounterFromTxAckAfterRx_Output_api2buf(UInt8Buffer* pDest , gpPd_GetFrameCounterFromTxAckAfterRx_Output_marshall_struct_t* pSourceoutput , gpPd_GetFrameCounterFromTxAckAfterRx_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetFrameCounterFromTxAckAfterRx_Input_buf2api(gpPd_SetFrameCounterFromTxAckAfterRx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetKeyIdFromTxAckAfterRx_Input_buf2api(gpPd_GetKeyIdFromTxAckAfterRx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetKeyIdFromTxAckAfterRx_Output_api2buf(UInt8Buffer* pDest , gpPd_GetKeyIdFromTxAckAfterRx_Output_marshall_struct_t* pSourceoutput , gpPd_GetKeyIdFromTxAckAfterRx_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_SetKeyIdFromTxAckAfterRx_Input_buf2api(gpPd_SetKeyIdFromTxAckAfterRx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpPd_GetTxChannel_Input_buf2api(gpPd_GetTxChannel_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetTxChannel_Output_api2buf(UInt8Buffer* pDest , gpPd_GetTxChannel_Output_marshall_struct_t* pSourceoutput , gpPd_GetTxChannel_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);

// Client functions
void gpPd_ReadByte_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , gpPd_Offset_t offset , UInt16* pIndex);
void gpPd_ReadByte_Output_buf2par(UInt8* returnVal , gpPd_Handle_t pdHandle , gpPd_Offset_t offset , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_WriteByte_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , gpPd_Offset_t offset , UInt8 byte , UInt16* pIndex);
void gpPd_ReadByteStream_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , gpPd_Offset_t offset , UInt8 length , UInt16* pIndex);
void gpPd_ReadByteStream_Output_buf2par(gpPd_Handle_t pdHandle , gpPd_Offset_t offset , UInt8 length , UInt8* pBuf , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_WriteByteStream_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , gpPd_Offset_t offset , UInt8 length , UInt8* pBuf , UInt16* pIndex);
void gpPd_GetPd_Output_buf2par(gpPd_Handle_t* returnVal , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetCustomPd_Input_par2buf(UInt8Buffer* pDest , gpPd_BufferType_t type , UInt16 size , UInt16* pIndex);
void gpPd_GetCustomPd_Output_buf2par(gpPd_Handle_t* returnVal , gpPd_BufferType_t type , UInt16 size , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_FreePd_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_cbDataConfirm_Input_par2buf(UInt8Buffer* pDest , UInt8 pbmHandle , UInt16 pbmOffset , UInt16 pbmLength , UInt16* pIndex);
void gpPd_cbDataConfirm_Output_buf2par(UInt8 pbmHandle , UInt16 pbmOffset , UInt16 pbmLength , gpPd_Loh_t* p_PdLoh , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_DataIndication_Input_par2buf(UInt8Buffer* pDest , UInt8 pbmHandle , UInt16 pbmOffset , UInt16 pbmLength , gpPd_BufferType_t type , UInt16* pIndex);
void gpPd_DataIndication_Output_buf2par(UInt8 pbmHandle , UInt16 pbmOffset , UInt16 pbmLength , gpPd_Loh_t* p_PdLoh , gpPd_BufferType_t type , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_DataRequest_Input_par2buf(UInt8Buffer* pDest , gpPd_Loh_t* p_PdLoh , UInt16* pIndex);
void gpPd_DataRequest_Output_buf2par(UInt8* returnVal , gpPd_Loh_t* p_PdLoh , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_cbPurgeConfirm_Input_par2buf(UInt8Buffer* pDest , UInt8 pbmHandle , UInt16* pIndex);
void gpPd_PurgeRequest_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_PurgeRequest_Output_buf2par(gpPd_Handle_t* returnVal , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SecRequest_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt8 dataOffset , UInt8 dataLength , UInt8 auxOffset , UInt8 auxLength , UInt16* pIndex);
void gpPd_SecRequest_Output_buf2par(UInt8* returnVal , gpPd_Handle_t pdHandle , UInt8 dataOffset , UInt8 dataLength , UInt8 auxOffset , UInt8 auxLength , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_cbSecConfirm_Input_par2buf(UInt8Buffer* pDest , UInt8 pbmHandle , UInt8 dataOffset , UInt8 dataLength , UInt16* pIndex);
void gpPd_cbSecConfirm_Output_buf2par(gpPd_Handle_t* returnVal , UInt8 pbmHandle , UInt8 dataOffset , UInt8 dataLength , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_CheckPdValid_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_CheckPdValid_Output_buf2par(gpPd_Result_t* returnVal , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetRssi_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetRssi_Output_buf2par(gpPd_Rssi_t* rssi , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetRssi_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , gpPd_Rssi_t rssi , UInt16* pIndex);
void gpPd_GetLqi_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetLqi_Output_buf2par(gpPd_Lqi_t* lqi , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetLqi_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , gpPd_Lqi_t lqi , UInt16* pIndex);
void gpPd_GetRxTimestamp_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetRxTimestamp_Output_buf2par(UInt32* timestamp , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetTxTimestamp_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetTxTimestamp_Output_buf2par(UInt32* timestamp , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetRxTimestamp_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , gpPd_TimeStamp_t timestamp , UInt16* pIndex);
void gpPd_SetTxTimestamp_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , gpPd_TimeStamp_t timestamp , UInt16* pIndex);
void gpPd_CopyPd_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_CopyPd_Output_buf2par(gpPd_Handle_t* copyPd , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetTxRetryCntr_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetTxRetryCntr_Output_buf2par(UInt8* retryCntr , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetRxTimestampChip_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetRxTimestampChip_Output_buf2par(gpPd_TimeStamp_t* timestamp , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_AppendWithUpdate_Input_par2buf(UInt8Buffer* pDest , gpPd_Loh_t* pPdLoh , UInt8 length , const UInt8* pData , UInt16* pIndex);
void gpPd_AppendWithUpdate_Output_buf2par(gpPd_Loh_t* pPdLoh , UInt8 length , const UInt8* pData , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_PrependWithUpdate_Input_par2buf(UInt8Buffer* pDest , gpPd_Loh_t* pPdLoh , UInt8 length , UInt8* pData , UInt16* pIndex);
void gpPd_PrependWithUpdate_Output_buf2par(gpPd_Loh_t* pPdLoh , UInt8 length , UInt8* pData , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_ReadWithUpdate_Input_par2buf(UInt8Buffer* pDest , gpPd_Loh_t* pPdLoh , UInt8 length , UInt16* pIndex);
void gpPd_ReadWithUpdate_Output_buf2par(gpPd_Loh_t* pPdLoh , UInt8 length , UInt8* pData , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetRxTimestampExt_Input_par2buf( UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex );  //FIXME - Manual
void gpPd_GetRxTimestampExt_Output_buf2par( gpPd_TimeStamp_t* timestamp , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex );  //FIXME - Manual
void gpPd_GetTxTimestampExt_Input_par2buf( UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex ); //FIXME - Manual
void gpPd_GetTxTimestampExt_Output_buf2par( gpPd_TimeStamp_t* timestamp , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex ); //FIXME - Manual
void gpPd_SetTxRetryCntr_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt8 retryCntr , UInt16* pIndex);
void gpPd_GetRxChannel_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetRxChannel_Output_buf2par(UInt8* rxChannel , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetRxChannel_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt8 rxChannel , UInt16* pIndex);
void gpPd_GetFramePendingAfterTx_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetFramePendingAfterTx_Output_buf2par(UInt8* framePending , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetFramePendingAfterTx_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt8 framePending , UInt16* pIndex);
void gpPd_GetTxCCACntr_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetTxCCACntr_Output_buf2par(UInt8* txCCACntr , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetTxAckLqi_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetTxAckLqi_Output_buf2par(gpPd_Lqi_t* ackLqi , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_GetFrameControlFromTxAckAfterRx_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetFrameControlFromTxAckAfterRx_Output_buf2par(UInt16* framePending , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetFrameControlFromTxAckAfterRx_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16 framePending , UInt16* pIndex);
void gpPd_GetRxEnhancedAckFromTxPbm_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetRxEnhancedAckFromTxPbm_Output_buf2par(Bool* enhancedAck , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetRxEnhancedAckFromTxPbm_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , Bool enhancedAck , UInt16* pIndex);
void gpPd_GetFrameCounterFromTxAckAfterRx_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetFrameCounterFromTxAckAfterRx_Output_buf2par(UInt32* frameCounter , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetFrameCounterFromTxAckAfterRx_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt32 frameCounter , UInt16* pIndex);
void gpPd_GetKeyIdFromTxAckAfterRx_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetKeyIdFromTxAckAfterRx_Output_buf2par(UInt8* keyId , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);
void gpPd_SetKeyIdFromTxAckAfterRx_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt8 keyId , UInt16* pIndex);
void gpPd_GetTxChannel_Input_par2buf(UInt8Buffer* pDest , gpPd_Handle_t pdHandle , UInt16* pIndex);
void gpPd_GetTxChannel_Output_buf2par(UInt8* channel , gpPd_Handle_t pdHandle , UInt8Buffer* pSource , UInt16* pIndex);

void gpPd_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPPD_MARSHALLING_H_


