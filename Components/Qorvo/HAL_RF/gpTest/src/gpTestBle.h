/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017, 2019, Qorvo Inc
 *
 * gpTestBle.h
 *
 *  The file is contains generic test functions, to be used in the Evaluation application.
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
 *
 */
#ifndef _GP_TESTBLE_H_
#define _GP_TESTBLE_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "gpTest.h"

//TX
#define gpTest_BleTxPhy1Mb                  gpHal_BleTxPhy1Mb
#define gpTest_BleTxPhy2Mb                  gpHal_BleTxPhy2Mb
#define gpTest_BleTxPhyCoded125kb           gpHal_BleTxPhyCoded125kb
#define gpTest_BleTxPhyCoded500kb           gpHal_BleTxPhyCoded500kb
#define gpTest_BleTxPhyInvalid              gpHal_BleTxPhyInvalid

//RX
#define gpTest_BleRxPhy1Mb                  gpHal_BleRxPhy1Mb
#define gpTest_BleRxPhy2Mb                  gpHal_BleRxPhy2Mb
#define gpTest_BleRxPhyCoded                gpHal_BleRxPhyCoded
#define gpTest_BleRxPhyInvalid              gpHal_BleRxPhyInvalid


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpTest_BleStart(void);
void gpTest_BleStop(void);
gpTest_Result_t gpTest_BleSetChannel(UInt8 channel);
UInt8 gpTest_BleGetChannel(void);
UInt8 gpTest_BleSetAntenna(UInt8 antenna);
UInt8 gpTest_BleSetTxPower (Int8 transmitPower);
Int8 gpTest_BleGetTxPower(void);
void gpTest_BleGetDeviceAddress(BtDeviceAddress_t* address);
void gpTest_BleTxPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions);

gpTest_AntennaSelection_t gpTest_BleGetTxAntenna(void);
gpTest_AntennaSelection_t gpTest_BleGetRxAntenna(void);


gpTest_ContinuousWaveMode_t gpTest_BleGetContinuousWaveMode(void);
void gpTest_BleSetContinuousWaveMode( gpTest_ContinuousWaveMode_t newMode );

void gpTest_BleSetRxState(Bool flag);
Bool gpTest_BleGetRxState(void);
UInt16 gpTest_BleGetNumberOfRxPackets(void);

void gpTest_BleSetModulation(gpTest_BleTxPhy_t modulation);
gpTest_Result_t gpTest_BleReceiverTest(void);
gpTest_Result_t gpTest_BleTransmitterTest(UInt8 length, UInt8 payloadtype);
UInt16 gpTest_BleTestEnd(void);

#endif // FILE_H
