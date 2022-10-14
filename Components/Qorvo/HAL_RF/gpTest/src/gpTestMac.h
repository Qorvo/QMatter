/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
 *
 * gpTest.c
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
#ifndef _GP_TESTMAC_H_
#define _GP_TESTMAC_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "gpTest.h"

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpTest_MacInit(void);
void gpTest_MacStart(void);
void gpTest_MacStop(void);
void gpTest_MacSetNumberOfRetries(UInt8 retries);
gpTest_Result_t gpTest_MacSetChannel(UInt8 channel);
UInt8 gpTest_MacGetChannel(void);
void gpTest_MacSetTxPower (Int8 transmitPower);
Int8 gpTest_MacGetTxPower(void);
void gpTest_MacSetContinuousWaveMode(gpTest_ContinuousWaveMode_t newMode);
void gpTest_MacSetPacketInPacketMode(Bool newPIP);
Bool gpTest_MacGetPacketInPacketMode(void);
void gpTest_MacGetSettings(gpTest_Settings_t * Settings);
void gpTest_MacSetTxAntenna(gpTest_AntennaSelection_t antenna);
void gpTest_MacSetRxAntenna(gpTest_AntennaSelection_t antenna);
gpTest_AntennaSelection_t gpTest_MacGetTxAntenna(void);
Bool gpTest_MacGetAntennaDiversity(void);
void gpTest_MacSetAntennaDiversity(Bool OnOff);
gpTest_ContinuousWaveMode_t gpTest_MacGetContinuousWaveMode(void);
Bool gpTest_MacGetContTx(void);
void gpTest_MacTxPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions);
void gpTest_MacTxCorruptedPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions);
void gpTest_MacTxPollPacket( UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions );
void gpTest_MacSetRxResponsePacket(UInt32 delayUs, UInt8 dataLength, UInt8* pData, UInt8 txOptions);
void gpTest_MacEDScan( UInt16 numberOfScans, UInt16 intervalInMs, UInt16 channelMask, UInt32 duration_us );
void gpTest_MacSetCollisionAvoidanceModeToUse( gpTest_CollisionAvoidanceMode_t newMode );
gpTest_CollisionAvoidanceMode_t gpTest_MacGetCollisionAvoidanceModeInUse(void);
UInt8 gpTest_MacGetAverageLQI(void);
Int8 gpTest_MacGetAverageRSSI(void);
void gpTest_MacGetStatistics(gpTest_Statistics_t *Statistics);
void gpTest_MacResetStatistics(void);

gpTest_Result_t gpTest_MacSetChannelForStack(UInt8 stackId, UInt8 channel);

Bool gpTest_MacGetPromiscuousMode(void);
UInt16 gpTest_MacGetPanId(gpTest_SourceIdentifier_t srcId);
UInt16 gpTest_MacGetShortAddress(gpTest_SourceIdentifier_t srcId);

void gpTest_MacSetMaxBE(UInt8 maxBE);
UInt8 gpTest_MacGetMaxBE(void);

void gpTest_MacSetMinBE(UInt8 minBE);
UInt8 gpTest_MacGetMinBE(void);

void gpTest_MacSetMaxCSMABackoffs(UInt8 maxBackoffs);
UInt8 gpTest_MacGetMaxCSMABackoffs(void);

void gpTest_MacSetRetransmitOnCcaFail(Bool enable);
Bool gpTest_MacGetRetransmitOnCcaFail(void);
void gpTest_MacSetRetransmitRandomBackoff(Bool enable);
Bool gpTest_MacGetRetransmitRandomBackoff(void);

void gpTest_MacSetMinBeRetransmit(UInt8 minBERetransmit);
UInt8 gpTest_MacGetMinBeRetransmit(void);
void gpTest_MacSetMaxBeRetransmit(UInt8 maxBERetransmit);
UInt8 gpTest_MacGetMaxBeRetransmit(void);

void gpTest_MacSetRxState( Bool flag  );
Bool gpTest_MacGetRxState(void);

UInt8 gpTest_MacGetNumberOfRetries( void );
void gpTest_MacSetNumberOfRetries( UInt8 retries );
void gpTest_MacSetExpectedRx(UInt8 dataLength, UInt8* pData);

gpTest_TxPower_t gpTest_MacGetLastUsedTxPower(void);
void gpTest_MacUnregisterCallbacks(void);
#endif // FILE_H
