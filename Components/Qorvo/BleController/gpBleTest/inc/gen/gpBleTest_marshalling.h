/*
 * Copyright (c) 2020, Qorvo Inc
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

#ifndef _GPBLETEST_MARSHALLING_H_
#define _GPBLETEST_MARSHALLING_H_

//DOCUMENTATION BLETEST: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpBleTest.h"
/* <CodeGenerator Placeholder> AdditionalIncludes */
#include "gpTest_marshalling.h"
#include "gpPd_marshalling.h"
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/



typedef struct {
    UInt8 length;
    UInt8 payloadtype;
} gpBleTest_TransmitterTest_Input_struct_t;

typedef struct {
    gpBleTest_TransmitterTest_Input_struct_t data;
} gpBleTest_TransmitterTest_Input_marshall_struct_t;

typedef struct {
    gpBleTest_Result_t result;
} gpBleTest_TransmitterTest_Output_struct_t;

typedef struct {
    gpBleTest_TransmitterTest_Output_struct_t data;
} gpBleTest_TransmitterTest_Output_marshall_struct_t;


typedef struct {
    gpBleTest_Result_t result;
} gpBleTest_ReceiverTest_Output_struct_t;

typedef struct {
    gpBleTest_ReceiverTest_Output_struct_t data;
} gpBleTest_ReceiverTest_Output_marshall_struct_t;


typedef struct {
    UInt16 result;
} gpBleTest_TestEnd_Output_struct_t;

typedef struct {
    gpBleTest_TestEnd_Output_struct_t data;
} gpBleTest_TestEnd_Output_marshall_struct_t;


typedef struct {
    gpBleTest_TxPhy_t txPhy;
} gpBleTest_SetTxPhy_Input_struct_t;

typedef struct {
    gpBleTest_SetTxPhy_Input_struct_t data;
} gpBleTest_SetTxPhy_Input_marshall_struct_t;


typedef struct {
    gpBleTest_TxPhy_t modulation;
} gpBleTest_SetModulation_Input_struct_t;

typedef struct {
    gpBleTest_SetModulation_Input_struct_t data;
} gpBleTest_SetModulation_Input_marshall_struct_t;


typedef struct {
    UInt8 rxMask;
} gpBleTest_SetRxPhyMask_Input_struct_t;

typedef struct {
    gpBleTest_SetRxPhyMask_Input_struct_t data;
} gpBleTest_SetRxPhyMask_Input_marshall_struct_t;

typedef struct {
    gpBleTest_Result_t result;
} gpBleTest_SetRxPhyMask_Output_struct_t;

typedef struct {
    gpBleTest_SetRxPhyMask_Output_struct_t data;
} gpBleTest_SetRxPhyMask_Output_marshall_struct_t;


typedef struct {
    BtDeviceAddress_t* btDeviceAddress;
} gpBleTest_GetDeviceAddress_Output_struct_t;

typedef struct {
    gpBleTest_GetDeviceAddress_Output_struct_t data;
    BtDeviceAddress_t btDeviceAddress[1];
} gpBleTest_GetDeviceAddress_Output_marshall_struct_t;


typedef struct {
    UInt16 gpTest_NumberOfRxPackets;
} gpBleTest_GetNumberOfRxPackets_Output_struct_t;

typedef struct {
    gpBleTest_GetNumberOfRxPackets_Output_struct_t data;
} gpBleTest_GetNumberOfRxPackets_Output_marshall_struct_t;


typedef struct {
    UInt16 number;
} gpBleTest_SetNumberTxPacketsInTestMode_Input_struct_t;

typedef struct {
    gpBleTest_SetNumberTxPacketsInTestMode_Input_struct_t data;
} gpBleTest_SetNumberTxPacketsInTestMode_Input_marshall_struct_t;


typedef struct {
    Bool enable;
} gpBleTest_EnableDtm_Input_struct_t;

typedef struct {
    gpBleTest_EnableDtm_Input_struct_t data;
} gpBleTest_EnableDtm_Input_marshall_struct_t;


typedef struct {
    UInt8 channel;
} gpBleTest_SetChannel_Input_struct_t;

typedef struct {
    gpBleTest_SetChannel_Input_struct_t data;
} gpBleTest_SetChannel_Input_marshall_struct_t;


typedef struct {
    UInt8 channel;
} gpBleTest_GetChannel_Output_struct_t;

typedef struct {
    gpBleTest_GetChannel_Output_struct_t data;
} gpBleTest_GetChannel_Output_marshall_struct_t;




typedef struct {
    BtDeviceAddress_t* btDeviceAddress;
} gpBleTest_SetDeviceAddress_Output_struct_t;

typedef struct {
    gpBleTest_SetDeviceAddress_Output_struct_t data;
    BtDeviceAddress_t btDeviceAddress[1];
} gpBleTest_SetDeviceAddress_Output_marshall_struct_t;


typedef struct {
    UInt8 power;
} gpBleTest_SetTxPower_Input_struct_t;

typedef struct {
    gpBleTest_SetTxPower_Input_struct_t data;
} gpBleTest_SetTxPower_Input_marshall_struct_t;

typedef struct {
    UInt8 status;
} gpBleTest_SetTxPower_Output_struct_t;

typedef struct {
    gpBleTest_SetTxPower_Output_struct_t data;
} gpBleTest_SetTxPower_Output_marshall_struct_t;


typedef struct {
    Int8 power;
} gpBleTest_GetTxPower_Output_struct_t;

typedef struct {
    gpBleTest_GetTxPower_Output_struct_t data;
} gpBleTest_GetTxPower_Output_marshall_struct_t;


typedef struct {
    gpBleTest_AntennaSelection_t antenna;
} gpBleTest_SetTxAntenna_Input_struct_t;

typedef struct {
    gpBleTest_SetTxAntenna_Input_struct_t data;
} gpBleTest_SetTxAntenna_Input_marshall_struct_t;


typedef struct {
    gpBleTest_AntennaSelection_t antenna;
} gpBleTest_GetTxAntenna_Output_struct_t;

typedef struct {
    gpBleTest_GetTxAntenna_Output_struct_t data;
} gpBleTest_GetTxAntenna_Output_marshall_struct_t;


typedef struct {
    gpBleTest_AntennaSelection_t antenna;
} gpBleTest_GetRxAntenna_Output_struct_t;

typedef struct {
    gpBleTest_GetRxAntenna_Output_struct_t data;
} gpBleTest_GetRxAntenna_Output_marshall_struct_t;



typedef struct {
    UInt8 newMode;
} gpBleTest_SetContinuousWaveMode_Input_struct_t;

typedef struct {
    gpBleTest_SetContinuousWaveMode_Input_struct_t data;
} gpBleTest_SetContinuousWaveMode_Input_marshall_struct_t;


typedef struct {
    UInt8 newMode;
} gpBleTest_GetContinuousWaveMode_Output_struct_t;

typedef struct {
    gpBleTest_GetContinuousWaveMode_Output_struct_t data;
} gpBleTest_GetContinuousWaveMode_Output_marshall_struct_t;


typedef struct {
    Bool flag;
} gpBleTest_SetRxState_Input_struct_t;

typedef struct {
    gpBleTest_SetRxState_Input_struct_t data;
} gpBleTest_SetRxState_Input_marshall_struct_t;


typedef struct {
    Bool rx_state;
} gpBleTest_GetRxState_Output_struct_t;

typedef struct {
    gpBleTest_GetRxState_Output_struct_t data;
} gpBleTest_GetRxState_Output_marshall_struct_t;


typedef union {
    gpBleTest_TransmitterTest_Input_marshall_struct_t gpBleTest_TransmitterTest;
    gpBleTest_SetTxPhy_Input_marshall_struct_t gpBleTest_SetTxPhy;
    gpBleTest_SetModulation_Input_marshall_struct_t gpBleTest_SetModulation;
    gpBleTest_SetRxPhyMask_Input_marshall_struct_t gpBleTest_SetRxPhyMask;
    gpBleTest_SetNumberTxPacketsInTestMode_Input_marshall_struct_t gpBleTest_SetNumberTxPacketsInTestMode;
    gpBleTest_EnableDtm_Input_marshall_struct_t gpBleTest_EnableDtm;
    gpBleTest_SetChannel_Input_marshall_struct_t gpBleTest_SetChannel;
    gpBleTest_SetTxPower_Input_marshall_struct_t gpBleTest_SetTxPower;
    gpBleTest_SetTxAntenna_Input_marshall_struct_t gpBleTest_SetTxAntenna;
    gpBleTest_SetContinuousWaveMode_Input_marshall_struct_t gpBleTest_SetContinuousWaveMode;
    gpBleTest_SetRxState_Input_marshall_struct_t gpBleTest_SetRxState;
    UInt8 dummy; //ensure none empty union definition
} gpBleTest_Server_Input_union_t;

typedef union {
    gpBleTest_TransmitterTest_Output_marshall_struct_t gpBleTest_TransmitterTest;
    gpBleTest_ReceiverTest_Output_marshall_struct_t gpBleTest_ReceiverTest;
    gpBleTest_TestEnd_Output_marshall_struct_t gpBleTest_TestEnd;
    gpBleTest_SetRxPhyMask_Output_marshall_struct_t gpBleTest_SetRxPhyMask;
    gpBleTest_GetDeviceAddress_Output_marshall_struct_t gpBleTest_GetDeviceAddress;
    gpBleTest_GetNumberOfRxPackets_Output_marshall_struct_t gpBleTest_GetNumberOfRxPackets;
    gpBleTest_GetChannel_Output_marshall_struct_t gpBleTest_GetChannel;
    gpBleTest_SetDeviceAddress_Output_marshall_struct_t gpBleTest_SetDeviceAddress;
    gpBleTest_SetTxPower_Output_marshall_struct_t gpBleTest_SetTxPower;
    gpBleTest_GetTxPower_Output_marshall_struct_t gpBleTest_GetTxPower;
    gpBleTest_GetTxAntenna_Output_marshall_struct_t gpBleTest_GetTxAntenna;
    gpBleTest_GetRxAntenna_Output_marshall_struct_t gpBleTest_GetRxAntenna;
    gpBleTest_GetContinuousWaveMode_Output_marshall_struct_t gpBleTest_GetContinuousWaveMode;
    gpBleTest_GetRxState_Output_marshall_struct_t gpBleTest_GetRxState;
    UInt8 dummy; //ensure none empty union definition
} gpBleTest_Server_Output_union_t;
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpBleTest_TxPhy_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpBleTest_TxPhy_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpBleTest_TxPhy_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpBleTest_TxPhy_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpBleTest_RxPhy_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpBleTest_RxPhy_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpBleTest_RxPhy_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpBleTest_RxPhy_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpBleTest_Result_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpBleTest_Result_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpBleTest_Result_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpBleTest_Result_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpBleTest_AntennaSelection_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpBleTest_AntennaSelection_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpBleTest_AntennaSelection_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpBleTest_AntennaSelection_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
// Server functions
gpMarshall_AckStatus_t gpBleTest_TransmitterTest_Input_buf2api(gpBleTest_TransmitterTest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_TransmitterTest_Output_api2buf(UInt8Buffer* pDest , gpBleTest_TransmitterTest_Output_marshall_struct_t* pSourceoutput , gpBleTest_TransmitterTest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpBleTest_ReceiverTest_Output_api2buf(UInt8Buffer* pDest , gpBleTest_ReceiverTest_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpBleTest_TestEnd_Output_api2buf(UInt8Buffer* pDest , gpBleTest_TestEnd_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetTxPhy_Input_buf2api(gpBleTest_SetTxPhy_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetModulation_Input_buf2api(gpBleTest_SetModulation_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetRxPhyMask_Input_buf2api(gpBleTest_SetRxPhyMask_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetRxPhyMask_Output_api2buf(UInt8Buffer* pDest , gpBleTest_SetRxPhyMask_Output_marshall_struct_t* pSourceoutput , gpBleTest_SetRxPhyMask_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpBleTest_GetDeviceAddress_Output_api2buf(UInt8Buffer* pDest , gpBleTest_GetDeviceAddress_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpBleTest_GetNumberOfRxPackets_Output_api2buf(UInt8Buffer* pDest , gpBleTest_GetNumberOfRxPackets_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetNumberTxPacketsInTestMode_Input_buf2api(gpBleTest_SetNumberTxPacketsInTestMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_EnableDtm_Input_buf2api(gpBleTest_EnableDtm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetChannel_Input_buf2api(gpBleTest_SetChannel_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_GetChannel_Output_api2buf(UInt8Buffer* pDest , gpBleTest_GetChannel_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpBleTest_SetDeviceAddress_Output_api2buf(UInt8Buffer* pDest , gpBleTest_SetDeviceAddress_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetTxPower_Input_buf2api(gpBleTest_SetTxPower_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetTxPower_Output_api2buf(UInt8Buffer* pDest , gpBleTest_SetTxPower_Output_marshall_struct_t* pSourceoutput , gpBleTest_SetTxPower_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpBleTest_GetTxPower_Output_api2buf(UInt8Buffer* pDest , gpBleTest_GetTxPower_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetTxAntenna_Input_buf2api(gpBleTest_SetTxAntenna_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_GetTxAntenna_Output_api2buf(UInt8Buffer* pDest , gpBleTest_GetTxAntenna_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpBleTest_GetRxAntenna_Output_api2buf(UInt8Buffer* pDest , gpBleTest_GetRxAntenna_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetContinuousWaveMode_Input_buf2api(gpBleTest_SetContinuousWaveMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_GetContinuousWaveMode_Output_api2buf(UInt8Buffer* pDest , gpBleTest_GetContinuousWaveMode_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpBleTest_SetRxState_Input_buf2api(gpBleTest_SetRxState_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_GetRxState_Output_api2buf(UInt8Buffer* pDest , gpBleTest_GetRxState_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);

// Client functions
void gpBleTest_TransmitterTest_Input_par2buf(UInt8Buffer* pDest , UInt8 length , UInt8 payloadtype , UInt16* pIndex);
void gpBleTest_TransmitterTest_Output_buf2par(gpBleTest_Result_t* result , UInt8 length , UInt8 payloadtype , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_ReceiverTest_Output_buf2par(gpBleTest_Result_t* result , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_TestEnd_Output_buf2par(UInt16* result , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetTxPhy_Input_par2buf(UInt8Buffer* pDest , gpBleTest_TxPhy_t txPhy , UInt16* pIndex);
void gpBleTest_SetModulation_Input_par2buf(UInt8Buffer* pDest , gpBleTest_TxPhy_t modulation , UInt16* pIndex);
void gpBleTest_SetRxPhyMask_Input_par2buf(UInt8Buffer* pDest , UInt8 rxMask , UInt16* pIndex);
void gpBleTest_SetRxPhyMask_Output_buf2par(gpBleTest_Result_t* result , UInt8 rxMask , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_GetDeviceAddress_Output_buf2par(BtDeviceAddress_t* btDeviceAddress , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_GetNumberOfRxPackets_Output_buf2par(UInt16* gpTest_NumberOfRxPackets , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetNumberTxPacketsInTestMode_Input_par2buf(UInt8Buffer* pDest , UInt16 number , UInt16* pIndex);
void gpBleTest_EnableDtm_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpBleTest_SetChannel_Input_par2buf(UInt8Buffer* pDest , UInt8 channel , UInt16* pIndex);
void gpBleTest_GetChannel_Output_buf2par(UInt8* channel , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetDeviceAddress_Output_buf2par(BtDeviceAddress_t* btDeviceAddress , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetTxPower_Input_par2buf(UInt8Buffer* pDest , UInt8 power , UInt16* pIndex);
void gpBleTest_SetTxPower_Output_buf2par(UInt8* status , UInt8 power , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_GetTxPower_Output_buf2par(Int8* power , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetTxAntenna_Input_par2buf(UInt8Buffer* pDest , gpBleTest_AntennaSelection_t antenna , UInt16* pIndex);
void gpBleTest_GetTxAntenna_Output_buf2par(gpBleTest_AntennaSelection_t* antenna , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_GetRxAntenna_Output_buf2par(gpBleTest_AntennaSelection_t* antenna , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetContinuousWaveMode_Input_par2buf(UInt8Buffer* pDest , UInt8 newMode , UInt16* pIndex);
void gpBleTest_GetContinuousWaveMode_Output_buf2par(UInt8* newMode , UInt8Buffer* pSource , UInt16* pIndex);
void gpBleTest_SetRxState_Input_par2buf(UInt8Buffer* pDest , Bool flag , UInt16* pIndex);
void gpBleTest_GetRxState_Output_buf2par(Bool* rx_state , UInt8Buffer* pSource , UInt16* pIndex);

void gpBleTest_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPBLETEST_MARSHALLING_H_


