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

/** @file "gpBleTest.h"
 *
 *  BLE Low level Test functions
 *
 *  Declarations of the public functions and enumerations of gpBleTest.
*/

#ifndef _GPBLETEST_H_
#define _GPBLETEST_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpPd.h"
#include "gpHal_Ble.h" // manual
#include "gpHal.h"     // manual
#include "gpPad.h"
#include "gpHal_Ble.h" // manual
#include "gpTest.h"
//#include "api.h"

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

typedef UInt8 gpBleTest_TxPhy_t;

typedef UInt8 gpBleTest_RxPhy_t;

/** @typedef gpBleTest_Result_t
 *  @brief Is actually 'gpHal_Result_t' but because of marshalling troubles sticking to UInt8
*/
typedef UInt8 gpBleTest_Result_t;

/** @typedef gpBleTest_AntennaSelection_t
 *  @brief Is actually 'gpBleTest_AntennaSelection_t' but because of marshalling troubles sticking to UInt8
*/
typedef UInt8 gpBleTest_AntennaSelection_t;

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
void gpBleTest_ResetRequest(void);

void gpBleTest_Init(void);

gpBleTest_Result_t gpBleTest_TransmitterTest(UInt8 length, UInt8 payloadtype);

gpBleTest_Result_t gpBleTest_ReceiverTest(void);

UInt16 gpBleTest_TestEnd(void);

void gpBleTest_SetTxPhy(gpBleTest_TxPhy_t txPhy);

void gpBleTest_SetModulation(gpBleTest_TxPhy_t modulation);

gpBleTest_Result_t gpBleTest_SetRxPhyMask(UInt8 rxMask);

void gpBleTest_GetDeviceAddress(BtDeviceAddress_t* btDeviceAddress);

UInt16 gpBleTest_GetNumberOfRxPackets(void);

void gpBleTest_SetNumberTxPacketsInTestMode(UInt16 number);

void gpBleTest_EnableDtm(Bool enable);

void gpBleTest_SetChannel(UInt8 channel);

UInt8 gpBleTest_GetChannel(void);

void gpBleTest_Start(void);

void gpBleTest_Stop(void);

void gpBleTest_SetDeviceAddress(BtDeviceAddress_t* btDeviceAddress);

UInt8 gpBleTest_SetTxPower(UInt8 power);

Int8 gpBleTest_GetTxPower(void);

void gpBleTest_SetTxAntenna(gpBleTest_AntennaSelection_t antenna);

gpBleTest_AntennaSelection_t gpBleTest_GetTxAntenna(void);

gpBleTest_AntennaSelection_t gpBleTest_GetRxAntenna(void);

void gpBleTest_AbortTestMode(void);

void gpBleTest_SetContinuousWaveMode(UInt8 newMode);

UInt8 gpBleTest_GetContinuousWaveMode(void);

void gpBleTest_SetRxState(Bool flag);

Bool gpBleTest_GetRxState(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPBLETEST_H_

