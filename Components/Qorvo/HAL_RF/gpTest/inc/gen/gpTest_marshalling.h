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

#ifndef _GPTEST_MARSHALLING_H_
#define _GPTEST_MARSHALLING_H_

//DOCUMENTATION TEST: no @file required as all documented items are refered to a group

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include <global.h>
#include "gpTest.h"
/* <CodeGenerator Placeholder> AdditionalIncludes */
#include "gpPd_marshalling.h"
#include "gpVersion_marshalling.h"
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    gpTest_Statistics_t data[1];
} gpTest_Statistics_t_l1_pointer_marshall_t;


typedef struct {
    gpTest_Settings_t data[1];
} gpTest_Settings_t_l1_pointer_marshall_t;


typedef struct {
    gpTest_CntPrio_t data[1];
} gpTest_CntPrio_t_l1_pointer_marshall_t;


typedef struct {
    gpTest_StatisticsCounter_t data[1];
} gpTest_StatisticsCounter_t_l1_pointer_marshall_t;



typedef struct {
    UInt16 numberOfPackets;
    UInt16 intervalInMs;
    UInt8 dataLength;
    UInt8* pData;
    UInt8 txOptions;
} gpTest_TxPacket_Input_struct_t;

typedef struct {
    gpTest_TxPacket_Input_struct_t data;
    UInt8 pData[GP_TEST_MAX_LENGTH_PACKET];
} gpTest_TxPacket_Input_marshall_struct_t;


typedef struct {
    UInt16 numberOfPackets;
    UInt16 intervalInMs;
    UInt8 dataLength;
    UInt8* pData;
    Bool randomData;
} gpTest_TxPollPacket_Input_struct_t;

typedef struct {
    gpTest_TxPollPacket_Input_struct_t data;
    UInt8 pData[GP_TEST_MAX_LENGTH_PACKET];
} gpTest_TxPollPacket_Input_marshall_struct_t;


typedef struct {
    UInt32 delayUs;
    UInt8 dataLength;
    UInt8* pData;
    Bool randomData;
} gpTest_SetRxResponsePacket_Input_struct_t;

typedef struct {
    gpTest_SetRxResponsePacket_Input_struct_t data;
    UInt8 pData[GP_TEST_MAX_LENGTH_PACKET];
} gpTest_SetRxResponsePacket_Input_marshall_struct_t;


typedef struct {
    UInt16 numberOfScans;
    UInt16 intervalInMs;
    UInt16 channelMask;
} gpTest_EDScan_Input_struct_t;

typedef struct {
    gpTest_EDScan_Input_struct_t data;
} gpTest_EDScan_Input_marshall_struct_t;


typedef struct {
    UInt16 numberOfScans;
    UInt16 intervalInMs;
    UInt16 channelMask;
    UInt32 duration_us;
} gpTest_ExtendedEDScan_Input_struct_t;

typedef struct {
    gpTest_ExtendedEDScan_Input_struct_t data;
} gpTest_ExtendedEDScan_Input_marshall_struct_t;



typedef struct {
    gpTest_CollisionAvoidanceMode_t newMode;
} gpTest_SetCollisionAvoidanceModeToUse_Input_struct_t;

typedef struct {
    gpTest_SetCollisionAvoidanceModeToUse_Input_struct_t data;
} gpTest_SetCollisionAvoidanceModeToUse_Input_marshall_struct_t;


typedef struct {
    gpTest_CollisionAvoidanceMode_t currentMode;
} gpTest_GetCollisionAvoidanceModeInUse_Output_struct_t;

typedef struct {
    gpTest_GetCollisionAvoidanceModeInUse_Output_struct_t data;
} gpTest_GetCollisionAvoidanceModeInUse_Output_marshall_struct_t;


typedef struct {
    Bool newPIP;
} gpTest_SetPacketInPacketMode_Input_struct_t;

typedef struct {
    gpTest_SetPacketInPacketMode_Input_struct_t data;
} gpTest_SetPacketInPacketMode_Input_marshall_struct_t;


typedef struct {
    Bool currentMode;
} gpTest_GetPacketInPacketMode_Output_struct_t;

typedef struct {
    gpTest_GetPacketInPacketMode_Output_struct_t data;
} gpTest_GetPacketInPacketMode_Output_marshall_struct_t;


typedef struct {
    Bool OnOff;
} gpTest_SetAntennaDiversity_Input_struct_t;

typedef struct {
    gpTest_SetAntennaDiversity_Input_struct_t data;
} gpTest_SetAntennaDiversity_Input_marshall_struct_t;


typedef struct {
    Bool AntennaDiversityOn;
} gpTest_GetAntennaDiversity_Output_struct_t;

typedef struct {
    gpTest_GetAntennaDiversity_Output_struct_t data;
} gpTest_GetAntennaDiversity_Output_marshall_struct_t;


typedef struct {
    gpTest_AntennaSelection_t antenna;
} gpTest_SetAntenna_Input_struct_t;

typedef struct {
    gpTest_SetAntenna_Input_struct_t data;
} gpTest_SetAntenna_Input_marshall_struct_t;

typedef struct {
    gpTest_Result_t result;
} gpTest_SetAntenna_Output_struct_t;

typedef struct {
    gpTest_SetAntenna_Output_struct_t data;
} gpTest_SetAntenna_Output_marshall_struct_t;


typedef struct {
    gpTest_Statistics_t* pStatistics;
} gpTest_GetStatistics_Output_struct_t;

typedef struct {
    gpTest_GetStatistics_Output_struct_t data;
    gpTest_Statistics_t_l1_pointer_marshall_t pStatistics;
} gpTest_GetStatistics_Output_marshall_struct_t;



typedef struct {
    gpTest_Settings_t* pSettings;
} gpTest_GetSettings_Output_struct_t;

typedef struct {
    gpTest_GetSettings_Output_struct_t data;
    gpTest_Settings_t_l1_pointer_marshall_t pSettings;
} gpTest_GetSettings_Output_marshall_struct_t;


typedef struct {
    UInt8 newChannel;
} gpTest_SetChannel_Input_struct_t;

typedef struct {
    gpTest_SetChannel_Input_struct_t data;
} gpTest_SetChannel_Input_marshall_struct_t;

typedef struct {
    gpTest_Result_t result;
} gpTest_SetChannel_Output_struct_t;

typedef struct {
    gpTest_SetChannel_Output_struct_t data;
} gpTest_SetChannel_Output_marshall_struct_t;


typedef struct {
    UInt8 currentChannel;
} gpTest_GetChannel_Output_struct_t;

typedef struct {
    gpTest_GetChannel_Output_struct_t data;
} gpTest_GetChannel_Output_marshall_struct_t;


typedef struct {
    gpTest_ContinuousWaveMode_t newMode;
} gpTest_SetContinuousWaveMode_Input_struct_t;

typedef struct {
    gpTest_SetContinuousWaveMode_Input_struct_t data;
} gpTest_SetContinuousWaveMode_Input_marshall_struct_t;


typedef struct {
    gpTest_ContinuousWaveMode_t currentMode;
} gpTest_GetContinuousWaveMode_Output_struct_t;

typedef struct {
    gpTest_GetContinuousWaveMode_Output_struct_t data;
} gpTest_GetContinuousWaveMode_Output_marshall_struct_t;


typedef struct {
    Bool ContTxEnabled;
} gpTest_GetContTx_Output_struct_t;

typedef struct {
    gpTest_GetContTx_Output_struct_t data;
} gpTest_GetContTx_Output_marshall_struct_t;


typedef struct {
    UInt8 lqi;
} gpTest_GetAverageLQI_Output_struct_t;

typedef struct {
    gpTest_GetAverageLQI_Output_struct_t data;
} gpTest_GetAverageLQI_Output_marshall_struct_t;


typedef struct {
    Int8 rssi;
} gpTest_GetAverageRSSI_Output_struct_t;

typedef struct {
    gpTest_GetAverageRSSI_Output_struct_t data;
} gpTest_GetAverageRSSI_Output_marshall_struct_t;


typedef struct {
    UInt8 maxBE;
} gpTest_SetMaxBE_Input_struct_t;

typedef struct {
    gpTest_SetMaxBE_Input_struct_t data;
} gpTest_SetMaxBE_Input_marshall_struct_t;


typedef struct {
    UInt8 maxBE;
} gpTest_GetMaxBE_Output_struct_t;

typedef struct {
    gpTest_GetMaxBE_Output_struct_t data;
} gpTest_GetMaxBE_Output_marshall_struct_t;


typedef struct {
    UInt8 minBE;
} gpTest_SetMinBE_Input_struct_t;

typedef struct {
    gpTest_SetMinBE_Input_struct_t data;
} gpTest_SetMinBE_Input_marshall_struct_t;


typedef struct {
    UInt8 minBE;
} gpTest_GetMinBE_Output_struct_t;

typedef struct {
    gpTest_GetMinBE_Output_struct_t data;
} gpTest_GetMinBE_Output_marshall_struct_t;


typedef struct {
    UInt8 maxBackoffs;
} gpTest_SetMaxCSMABackoffs_Input_struct_t;

typedef struct {
    gpTest_SetMaxCSMABackoffs_Input_struct_t data;
} gpTest_SetMaxCSMABackoffs_Input_marshall_struct_t;


typedef struct {
    UInt8 maxBackoffs;
} gpTest_GetMaxCSMABackoffs_Output_struct_t;

typedef struct {
    gpTest_GetMaxCSMABackoffs_Output_struct_t data;
} gpTest_GetMaxCSMABackoffs_Output_marshall_struct_t;


typedef struct {
    UInt8 retries;
} gpTest_SetNumberOfRetries_Input_struct_t;

typedef struct {
    gpTest_SetNumberOfRetries_Input_struct_t data;
} gpTest_SetNumberOfRetries_Input_marshall_struct_t;


typedef struct {
    UInt8 retries;
} gpTest_GetNumberOfRetries_Output_struct_t;

typedef struct {
    gpTest_GetNumberOfRetries_Output_struct_t data;
} gpTest_GetNumberOfRetries_Output_marshall_struct_t;


typedef struct {
    UInt16 panId;
    gpTest_SourceIdentifier_t srcId;
} gpTest_SetPanId_Input_struct_t;

typedef struct {
    gpTest_SetPanId_Input_struct_t data;
} gpTest_SetPanId_Input_marshall_struct_t;


typedef struct {
    gpTest_SourceIdentifier_t srcId;
} gpTest_GetPanId_Input_struct_t;

typedef struct {
    gpTest_GetPanId_Input_struct_t data;
} gpTest_GetPanId_Input_marshall_struct_t;

typedef struct {
    UInt16 panId;
} gpTest_GetPanId_Output_struct_t;

typedef struct {
    gpTest_GetPanId_Output_struct_t data;
} gpTest_GetPanId_Output_marshall_struct_t;


typedef struct {
    Bool flag;
} gpTest_SetPromiscuousMode_Input_struct_t;

typedef struct {
    gpTest_SetPromiscuousMode_Input_struct_t data;
} gpTest_SetPromiscuousMode_Input_marshall_struct_t;


typedef struct {
    Bool flag;
} gpTest_GetPromiscuousMode_Output_struct_t;

typedef struct {
    gpTest_GetPromiscuousMode_Output_struct_t data;
} gpTest_GetPromiscuousMode_Output_marshall_struct_t;


typedef struct {
    gpTest_AntennaSelection_t antenna;
} gpTest_GetRxAntenna_Output_struct_t;

typedef struct {
    gpTest_GetRxAntenna_Output_struct_t data;
} gpTest_GetRxAntenna_Output_marshall_struct_t;


typedef struct {
    Bool rxOn;
} gpTest_SetRxState_Input_struct_t;

typedef struct {
    gpTest_SetRxState_Input_struct_t data;
} gpTest_SetRxState_Input_marshall_struct_t;


typedef struct {
    Bool rxOn;
} gpTest_GetRxState_Output_struct_t;

typedef struct {
    gpTest_GetRxState_Output_struct_t data;
} gpTest_GetRxState_Output_marshall_struct_t;


typedef struct {
    UInt16 shortAddress;
    gpTest_SourceIdentifier_t srcId;
} gpTest_SetShortAddress_Input_struct_t;

typedef struct {
    gpTest_SetShortAddress_Input_struct_t data;
} gpTest_SetShortAddress_Input_marshall_struct_t;


typedef struct {
    gpTest_SourceIdentifier_t srcId;
} gpTest_GetShortAddress_Input_struct_t;

typedef struct {
    gpTest_GetShortAddress_Input_struct_t data;
} gpTest_GetShortAddress_Input_marshall_struct_t;

typedef struct {
    UInt16 shortAddress;
} gpTest_GetShortAddress_Output_struct_t;

typedef struct {
    gpTest_GetShortAddress_Output_struct_t data;
} gpTest_GetShortAddress_Output_marshall_struct_t;


typedef struct {
    Int8 transmitPower;
} gpTest_SetTxPower_Input_struct_t;

typedef struct {
    gpTest_SetTxPower_Input_struct_t data;
} gpTest_SetTxPower_Input_marshall_struct_t;


typedef struct {
    Int8 transmitPower;
} gpTest_GetTxPower_Output_struct_t;

typedef struct {
    gpTest_GetTxPower_Output_struct_t data;
} gpTest_GetTxPower_Output_marshall_struct_t;


typedef struct {
    MACAddress_t* pExtendedAddress;
} gpTest_SetExtendedAddress_Input_struct_t;

typedef struct {
    gpTest_SetExtendedAddress_Input_struct_t data;
    MACAddress_t pExtendedAddress[1];
} gpTest_SetExtendedAddress_Input_marshall_struct_t;


typedef struct {
    MACAddress_t* pExtendedAddress;
} gpTest_GetExtendedAddress_Output_struct_t;

typedef struct {
    gpTest_GetExtendedAddress_Output_struct_t data;
    MACAddress_t pExtendedAddress[1];
} gpTest_GetExtendedAddress_Output_marshall_struct_t;


typedef struct {
    Bool enable;
    Bool panCoordinator;
} gpTest_SetAddressRecognition_Input_struct_t;

typedef struct {
    gpTest_SetAddressRecognition_Input_struct_t data;
} gpTest_SetAddressRecognition_Input_marshall_struct_t;


typedef struct {
    Bool addrRecognitionFlag;
} gpTest_GetAddressRecognition_Output_struct_t;

typedef struct {
    gpTest_GetAddressRecognition_Output_struct_t data;
} gpTest_GetAddressRecognition_Output_marshall_struct_t;


typedef struct {
    gpTest_AntennaSelection_t antenna;
} gpTest_GetTxAntenna_Output_struct_t;

typedef struct {
    gpTest_GetTxAntenna_Output_struct_t data;
} gpTest_GetTxAntenna_Output_marshall_struct_t;


typedef struct {
    gpTest_VersioningIndex_t versionType;
} gpTest_GetVersionInfo_Input_struct_t;

typedef struct {
    gpTest_GetVersionInfo_Input_struct_t data;
} gpTest_GetVersionInfo_Input_marshall_struct_t;

typedef struct {
    gpVersion_ReleaseInfo_t* pVersion;
} gpTest_GetVersionInfo_Output_struct_t;

typedef struct {
    gpTest_GetVersionInfo_Output_struct_t data;
    gpVersion_ReleaseInfo_t_l1_pointer_marshall_t pVersion;
} gpTest_GetVersionInfo_Output_marshall_struct_t;


typedef struct {
    UInt32 address;
} gpTest_ReadReg_Input_struct_t;

typedef struct {
    gpTest_ReadReg_Input_struct_t data;
} gpTest_ReadReg_Input_marshall_struct_t;

typedef struct {
    UInt8 readByte;
} gpTest_ReadReg_Output_struct_t;

typedef struct {
    gpTest_ReadReg_Output_struct_t data;
} gpTest_ReadReg_Output_marshall_struct_t;


typedef struct {
    UInt32 address;
    UInt8 writeByte;
} gpTest_WriteReg_Input_struct_t;

typedef struct {
    gpTest_WriteReg_Input_struct_t data;
} gpTest_WriteReg_Input_marshall_struct_t;

typedef struct {
    Bool successStatus;
} gpTest_WriteReg_Output_struct_t;

typedef struct {
    gpTest_WriteReg_Output_struct_t data;
} gpTest_WriteReg_Output_marshall_struct_t;


typedef struct {
    Bool enable;
} gpTest_PrintfEnable_Input_struct_t;

typedef struct {
    gpTest_PrintfEnable_Input_struct_t data;
} gpTest_PrintfEnable_Input_marshall_struct_t;


typedef struct {
    gpTest_SleepMode_t mode;
} gpTest_SetSleepMode_Input_struct_t;

typedef struct {
    gpTest_SetSleepMode_Input_struct_t data;
} gpTest_SetSleepMode_Input_marshall_struct_t;


typedef struct {
    gpTest_SleepMode_t mode;
} gpTest_GetSleepMode_Output_struct_t;

typedef struct {
    gpTest_GetSleepMode_Output_struct_t data;
} gpTest_GetSleepMode_Output_marshall_struct_t;




typedef struct {
    Bool isAwake;
} gpTest_IsAwake_Output_struct_t;

typedef struct {
    gpTest_IsAwake_Output_struct_t data;
} gpTest_IsAwake_Output_marshall_struct_t;



typedef struct {
    Bool enable;
} gpTest_EnableExternalLna_Input_struct_t;

typedef struct {
    gpTest_EnableExternalLna_Input_struct_t data;
} gpTest_EnableExternalLna_Input_marshall_struct_t;


typedef struct {
    gpTest_TxPower_t dBm;
} gpTest_GetLastUsedTxPower_Output_struct_t;

typedef struct {
    gpTest_GetLastUsedTxPower_Output_struct_t data;
} gpTest_GetLastUsedTxPower_Output_marshall_struct_t;


typedef struct {
    gpTest_PhyMode_t PhyMode;
} gpTest_SetPhyMode_Input_struct_t;

typedef struct {
    gpTest_SetPhyMode_Input_struct_t data;
} gpTest_SetPhyMode_Input_marshall_struct_t;


typedef struct {
    gpTest_PhyMode_t phyMode;
} gpTest_GetPhyMode_Output_struct_t;

typedef struct {
    gpTest_GetPhyMode_Output_struct_t data;
} gpTest_GetPhyMode_Output_marshall_struct_t;




typedef struct {
    Bool enabled;
} gpTest_CheckDCDCEnable_Output_struct_t;

typedef struct {
    gpTest_CheckDCDCEnable_Output_struct_t data;
} gpTest_CheckDCDCEnable_Output_marshall_struct_t;


#if defined(GP_COMP_GPTEST_BLE)
typedef struct {
    UInt8 length;
    UInt8 payloadtype;
} gpTest_BleTransmitterTest_Input_struct_t;

typedef struct {
    gpTest_BleTransmitterTest_Input_struct_t data;
} gpTest_BleTransmitterTest_Input_marshall_struct_t;

typedef struct {
    gpTest_Result_t result;
} gpTest_BleTransmitterTest_Output_struct_t;

typedef struct {
    gpTest_BleTransmitterTest_Output_struct_t data;
} gpTest_BleTransmitterTest_Output_marshall_struct_t;

#endif /* defined(GP_COMP_GPTEST_BLE) */

#if defined(GP_COMP_GPTEST_BLE)
typedef struct {
    gpTest_Result_t result;
} gpTest_BleReceiverTest_Output_struct_t;

typedef struct {
    gpTest_BleReceiverTest_Output_struct_t data;
} gpTest_BleReceiverTest_Output_marshall_struct_t;

#endif /* defined(GP_COMP_GPTEST_BLE) */

#if defined(GP_COMP_GPTEST_BLE)
typedef struct {
    UInt16 result;
} gpTest_BleTestEnd_Output_struct_t;

typedef struct {
    gpTest_BleTestEnd_Output_struct_t data;
} gpTest_BleTestEnd_Output_marshall_struct_t;

#endif /* defined(GP_COMP_GPTEST_BLE) */

#if defined(GP_COMP_GPTEST_BLE)
typedef struct {
    gpTest_BleTxPhy_t modulation;
} gpTest_SetModulation_Input_struct_t;

typedef struct {
    gpTest_SetModulation_Input_struct_t data;
} gpTest_SetModulation_Input_marshall_struct_t;

#endif /* defined(GP_COMP_GPTEST_BLE) */

typedef struct {
    UInt8 clockspeed;
} gpTest_SetMcuClockSpeed_Input_struct_t;

typedef struct {
    gpTest_SetMcuClockSpeed_Input_struct_t data;
} gpTest_SetMcuClockSpeed_Input_marshall_struct_t;

typedef struct {
    gpTest_Result_t result;
} gpTest_SetMcuClockSpeed_Output_struct_t;

typedef struct {
    gpTest_SetMcuClockSpeed_Output_struct_t data;
} gpTest_SetMcuClockSpeed_Output_marshall_struct_t;


typedef struct {
    UInt16 numberOfRestarts;
    UInt32 intervalInUs;
    UInt32 stopDurationInUs;
    UInt32 delayUs;
    UInt8 trigger;
} gpTest_IpcRestart_Input_struct_t;

typedef struct {
    gpTest_IpcRestart_Input_struct_t data;
} gpTest_IpcRestart_Input_marshall_struct_t;

typedef struct {
    gpTest_Result_t result;
} gpTest_IpcRestart_Output_struct_t;

typedef struct {
    gpTest_IpcRestart_Output_struct_t data;
} gpTest_IpcRestart_Output_marshall_struct_t;


typedef struct {
    gpTest_SleepMode_t mode;
} gpTest_GetMeasuredSleepClockFrequency_Input_struct_t;

typedef struct {
    gpTest_GetMeasuredSleepClockFrequency_Input_struct_t data;
} gpTest_GetMeasuredSleepClockFrequency_Input_marshall_struct_t;

typedef struct {
    gpTest_SleepClockMeasurementStatus_t status;
    UInt32* frequencymHz;
} gpTest_GetMeasuredSleepClockFrequency_Output_struct_t;

typedef struct {
    gpTest_GetMeasuredSleepClockFrequency_Output_struct_t data;
    UInt32 frequencymHz[1];
} gpTest_GetMeasuredSleepClockFrequency_Output_marshall_struct_t;


typedef struct {
    UInt16 numberOfPackets;
    UInt16 intervalInMs;
    UInt8 dataLength;
    UInt8* pData;
    UInt8 txOptions;
} gpTest_TxCorruptedPacket_Input_struct_t;

typedef struct {
    gpTest_TxCorruptedPacket_Input_struct_t data;
    UInt8 pData[GP_TEST_MAX_LENGTH_PACKET];
} gpTest_TxCorruptedPacket_Input_marshall_struct_t;


typedef struct {
    UInt8 chipid;
} gpTest_GetChipId_Output_struct_t;

typedef struct {
    gpTest_GetChipId_Output_struct_t data;
} gpTest_GetChipId_Output_marshall_struct_t;


typedef struct {
    UInt8 chipversion;
} gpTest_GetChipVersion_Output_struct_t;

typedef struct {
    gpTest_GetChipVersion_Output_struct_t data;
} gpTest_GetChipVersion_Output_marshall_struct_t;


#if defined(GP_COMP_GPTEST_BLE)
typedef struct {
    BtDeviceAddress_t* btDeviceAddress;
} gpTest_BleGetDeviceAddress_Output_struct_t;

typedef struct {
    gpTest_BleGetDeviceAddress_Output_struct_t data;
    BtDeviceAddress_t btDeviceAddress[1];
} gpTest_BleGetDeviceAddress_Output_marshall_struct_t;

#endif /* defined(GP_COMP_GPTEST_BLE) */

#if defined(GP_COMP_GPTEST_BLE)
typedef struct {
    UInt16 gpTest_NumberOfRxPackets;
} gpTest_BleGetNumberOfRxPackets_Output_struct_t;

typedef struct {
    gpTest_BleGetNumberOfRxPackets_Output_struct_t data;
} gpTest_BleGetNumberOfRxPackets_Output_marshall_struct_t;

#endif /* defined(GP_COMP_GPTEST_BLE) */

#if defined(GP_COMP_GPTEST_BLE)
typedef struct {
    UInt16 number;
} gpTest_BleSetNumberTxPacketsInTestMode_Input_struct_t;

typedef struct {
    gpTest_BleSetNumberTxPacketsInTestMode_Input_struct_t data;
} gpTest_BleSetNumberTxPacketsInTestMode_Input_marshall_struct_t;

#endif /* defined(GP_COMP_GPTEST_BLE) */

typedef struct {
    gpTest_StatisticsCounter_t* pStatisticsCounters;
} gpTest_StatisticsCountersGet_Output_struct_t;

typedef struct {
    gpTest_StatisticsCountersGet_Output_struct_t data;
    gpTest_StatisticsCounter_t_l1_pointer_marshall_t pStatisticsCounters;
} gpTest_StatisticsCountersGet_Output_marshall_struct_t;



typedef struct {
    Bool flag;
} gpTest_SetSnifferMode_Input_struct_t;

typedef struct {
    gpTest_SetSnifferMode_Input_struct_t data;
} gpTest_SetSnifferMode_Input_marshall_struct_t;


typedef struct {
    Bool enable;
} gpTest_SetPwrCtrlInByPassMode_Input_struct_t;

typedef struct {
    gpTest_SetPwrCtrlInByPassMode_Input_struct_t data;
} gpTest_SetPwrCtrlInByPassMode_Input_marshall_struct_t;


typedef struct {
    Bool gpTest_PwrCtrlInByPassMode;
} gpTest_GetPwrCtrlInByPassMode_Output_struct_t;

typedef struct {
    gpTest_GetPwrCtrlInByPassMode_Output_struct_t data;
} gpTest_GetPwrCtrlInByPassMode_Output_marshall_struct_t;


typedef struct {
    UInt8* productId;
} gpTest_ReadProductId_Output_struct_t;

typedef struct {
    gpTest_ReadProductId_Output_struct_t data;
    UInt8 productId[10];
} gpTest_ReadProductId_Output_marshall_struct_t;


typedef struct {
    Bool enable;
} gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode_Input_struct_t;

typedef struct {
    gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode_Input_struct_t data;
} gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode_Input_marshall_struct_t;


typedef struct {
    UInt8 packetsBuffered;
} gpTest_SetDpiZbBuffering_Input_struct_t;

typedef struct {
    gpTest_SetDpiZbBuffering_Input_struct_t data;
} gpTest_SetDpiZbBuffering_Input_marshall_struct_t;


typedef struct {
    Bool enable;
} gpTest_EnableDpiZb_Input_struct_t;

typedef struct {
    gpTest_EnableDpiZb_Input_struct_t data;
} gpTest_EnableDpiZb_Input_marshall_struct_t;


typedef struct {
    Bool enable;
} gpTest_EnableDtm_Input_struct_t;

typedef struct {
    gpTest_EnableDtm_Input_struct_t data;
} gpTest_EnableDtm_Input_marshall_struct_t;


typedef struct {
    Bool enableMultiStandard;
    Bool enableMultiChannel;
    Bool enableHighSensitivity;
} gpTest_SetRxModeOptions_Input_struct_t;

typedef struct {
    gpTest_SetRxModeOptions_Input_struct_t data;
} gpTest_SetRxModeOptions_Input_marshall_struct_t;

typedef struct {
    gpTest_Result_t result;
} gpTest_SetRxModeOptions_Output_struct_t;

typedef struct {
    gpTest_SetRxModeOptions_Output_struct_t data;
} gpTest_SetRxModeOptions_Output_marshall_struct_t;


typedef struct {
    UInt8 stack1_channel;
    UInt8 stack2_channel;
} gpTest_SetChannelForOtherStacks_Input_struct_t;

typedef struct {
    gpTest_SetChannelForOtherStacks_Input_struct_t data;
} gpTest_SetChannelForOtherStacks_Input_marshall_struct_t;

typedef struct {
    gpTest_Result_t result;
} gpTest_SetChannelForOtherStacks_Output_struct_t;

typedef struct {
    gpTest_SetChannelForOtherStacks_Output_struct_t data;
} gpTest_SetChannelForOtherStacks_Output_marshall_struct_t;


typedef struct {
    UInt8 dataLength;
    UInt8* pData;
} gpTest_MacSetExpectedRx_Input_struct_t;

typedef struct {
    gpTest_MacSetExpectedRx_Input_struct_t data;
    UInt8 pData[GP_TEST_MAX_LENGTH_PACKET];
} gpTest_MacSetExpectedRx_Input_marshall_struct_t;


typedef struct {
    Bool enable;
} gpTest_SetRetransmitOnCcaFail_Input_struct_t;

typedef struct {
    gpTest_SetRetransmitOnCcaFail_Input_struct_t data;
} gpTest_SetRetransmitOnCcaFail_Input_marshall_struct_t;


typedef struct {
    Bool gpTest_RetransmitOnCcaFail;
} gpTest_GetRetransmitOnCcaFail_Output_struct_t;

typedef struct {
    gpTest_GetRetransmitOnCcaFail_Output_struct_t data;
} gpTest_GetRetransmitOnCcaFail_Output_marshall_struct_t;


typedef struct {
    Bool enable;
} gpTest_SetRetransmitRandomBackoff_Input_struct_t;

typedef struct {
    gpTest_SetRetransmitRandomBackoff_Input_struct_t data;
} gpTest_SetRetransmitRandomBackoff_Input_marshall_struct_t;


typedef struct {
    Bool gpTest_RetransmitRandomBackoff;
} gpTest_GetRetransmitRandomBackoff_Output_struct_t;

typedef struct {
    gpTest_GetRetransmitRandomBackoff_Output_struct_t data;
} gpTest_GetRetransmitRandomBackoff_Output_marshall_struct_t;


typedef struct {
    UInt8 minBERetransmit;
} gpTest_SetMinBeRetransmit_Input_struct_t;

typedef struct {
    gpTest_SetMinBeRetransmit_Input_struct_t data;
} gpTest_SetMinBeRetransmit_Input_marshall_struct_t;


typedef struct {
    UInt8 gpTest_MinBeRetransmit;
} gpTest_GetMinBeRetransmit_Output_struct_t;

typedef struct {
    gpTest_GetMinBeRetransmit_Output_struct_t data;
} gpTest_GetMinBeRetransmit_Output_marshall_struct_t;


typedef struct {
    UInt8 maxBERetransmit;
} gpTest_SetMaxBeRetransmit_Input_struct_t;

typedef struct {
    gpTest_SetMaxBeRetransmit_Input_struct_t data;
} gpTest_SetMaxBeRetransmit_Input_marshall_struct_t;


typedef struct {
    UInt8 gpTest_MaxBeRetransmit;
} gpTest_GetMaxBeRetransmit_Output_struct_t;

typedef struct {
    gpTest_GetMaxBeRetransmit_Output_struct_t data;
} gpTest_GetMaxBeRetransmit_Output_marshall_struct_t;


typedef struct {
    UInt8 status;
    UInt16 packetsSentOK;
    UInt16 packetsSentError;
} gpTest_cbDataConfirm_Input_struct_t;

typedef struct {
    gpTest_cbDataConfirm_Input_struct_t data;
} gpTest_cbDataConfirm_Input_marshall_struct_t;


typedef struct {
    UInt8 length;
    gpPd_Offset_t dataOffset;
    gpPd_Handle_t handle;
} gpTest_cbDataIndication_Input_struct_t;

typedef struct {
    gpTest_cbDataIndication_Input_struct_t data;
} gpTest_cbDataIndication_Input_marshall_struct_t;


typedef struct {
    UInt8 result;
    UInt16 channelMask;
    UInt8* pData;
    Bool Finished;
} gpTest_cbEDConfirm_Input_struct_t;

typedef struct {
    gpTest_cbEDConfirm_Input_struct_t data;
    UInt8 pData[16];
} gpTest_cbEDConfirm_Input_marshall_struct_t;


#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
typedef struct {
    UInt8 result;
    UInt32 stopDurationUs;
    UInt32 restartDurationUs;
} gpTest_cbIpcRestartConfirm_Input_struct_t;

typedef struct {
    gpTest_cbIpcRestartConfirm_Input_struct_t data;
} gpTest_cbIpcRestartConfirm_Input_marshall_struct_t;

#endif /* defined(GP_HAL_DIVERSITY_INCLUDE_IPC) */

typedef union {
    gpTest_TxPacket_Input_marshall_struct_t gpTest_TxPacket;
    gpTest_TxPollPacket_Input_marshall_struct_t gpTest_TxPollPacket;
    gpTest_SetRxResponsePacket_Input_marshall_struct_t gpTest_SetRxResponsePacket;
    gpTest_EDScan_Input_marshall_struct_t gpTest_EDScan;
    gpTest_ExtendedEDScan_Input_marshall_struct_t gpTest_ExtendedEDScan;
    gpTest_SetCollisionAvoidanceModeToUse_Input_marshall_struct_t gpTest_SetCollisionAvoidanceModeToUse;
    gpTest_SetPacketInPacketMode_Input_marshall_struct_t gpTest_SetPacketInPacketMode;
    gpTest_SetAntennaDiversity_Input_marshall_struct_t gpTest_SetAntennaDiversity;
    gpTest_SetAntenna_Input_marshall_struct_t gpTest_SetAntenna;
    gpTest_SetChannel_Input_marshall_struct_t gpTest_SetChannel;
    gpTest_SetContinuousWaveMode_Input_marshall_struct_t gpTest_SetContinuousWaveMode;
    gpTest_SetMaxBE_Input_marshall_struct_t gpTest_SetMaxBE;
    gpTest_SetMinBE_Input_marshall_struct_t gpTest_SetMinBE;
    gpTest_SetMaxCSMABackoffs_Input_marshall_struct_t gpTest_SetMaxCSMABackoffs;
    gpTest_SetNumberOfRetries_Input_marshall_struct_t gpTest_SetNumberOfRetries;
    gpTest_SetPanId_Input_marshall_struct_t gpTest_SetPanId;
    gpTest_GetPanId_Input_marshall_struct_t gpTest_GetPanId;
    gpTest_SetPromiscuousMode_Input_marshall_struct_t gpTest_SetPromiscuousMode;
    gpTest_SetRxState_Input_marshall_struct_t gpTest_SetRxState;
    gpTest_SetShortAddress_Input_marshall_struct_t gpTest_SetShortAddress;
    gpTest_GetShortAddress_Input_marshall_struct_t gpTest_GetShortAddress;
    gpTest_SetTxPower_Input_marshall_struct_t gpTest_SetTxPower;
    gpTest_SetExtendedAddress_Input_marshall_struct_t gpTest_SetExtendedAddress;
    gpTest_SetAddressRecognition_Input_marshall_struct_t gpTest_SetAddressRecognition;
    gpTest_GetVersionInfo_Input_marshall_struct_t gpTest_GetVersionInfo;
    gpTest_ReadReg_Input_marshall_struct_t gpTest_ReadReg;
    gpTest_WriteReg_Input_marshall_struct_t gpTest_WriteReg;
    gpTest_PrintfEnable_Input_marshall_struct_t gpTest_PrintfEnable;
    gpTest_SetSleepMode_Input_marshall_struct_t gpTest_SetSleepMode;
    gpTest_EnableExternalLna_Input_marshall_struct_t gpTest_EnableExternalLna;
    gpTest_SetPhyMode_Input_marshall_struct_t gpTest_SetPhyMode;
#if defined(GP_COMP_GPTEST_BLE)
    gpTest_BleTransmitterTest_Input_marshall_struct_t gpTest_BleTransmitterTest;
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
    gpTest_SetModulation_Input_marshall_struct_t gpTest_SetModulation;
#endif /* defined(GP_COMP_GPTEST_BLE) */
    gpTest_SetMcuClockSpeed_Input_marshall_struct_t gpTest_SetMcuClockSpeed;
    gpTest_IpcRestart_Input_marshall_struct_t gpTest_IpcRestart;
    gpTest_GetMeasuredSleepClockFrequency_Input_marshall_struct_t gpTest_GetMeasuredSleepClockFrequency;
    gpTest_TxCorruptedPacket_Input_marshall_struct_t gpTest_TxCorruptedPacket;
#if defined(GP_COMP_GPTEST_BLE)
    gpTest_BleSetNumberTxPacketsInTestMode_Input_marshall_struct_t gpTest_BleSetNumberTxPacketsInTestMode;
#endif /* defined(GP_COMP_GPTEST_BLE) */
    gpTest_SetSnifferMode_Input_marshall_struct_t gpTest_SetSnifferMode;
    gpTest_SetPwrCtrlInByPassMode_Input_marshall_struct_t gpTest_SetPwrCtrlInByPassMode;
    gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode_Input_marshall_struct_t gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode;
    gpTest_SetDpiZbBuffering_Input_marshall_struct_t gpTest_SetDpiZbBuffering;
    gpTest_EnableDpiZb_Input_marshall_struct_t gpTest_EnableDpiZb;
    gpTest_EnableDtm_Input_marshall_struct_t gpTest_EnableDtm;
    gpTest_SetRxModeOptions_Input_marshall_struct_t gpTest_SetRxModeOptions;
    gpTest_SetChannelForOtherStacks_Input_marshall_struct_t gpTest_SetChannelForOtherStacks;
    gpTest_MacSetExpectedRx_Input_marshall_struct_t gpTest_MacSetExpectedRx;
    gpTest_SetRetransmitOnCcaFail_Input_marshall_struct_t gpTest_SetRetransmitOnCcaFail;
    gpTest_SetRetransmitRandomBackoff_Input_marshall_struct_t gpTest_SetRetransmitRandomBackoff;
    gpTest_SetMinBeRetransmit_Input_marshall_struct_t gpTest_SetMinBeRetransmit;
    gpTest_SetMaxBeRetransmit_Input_marshall_struct_t gpTest_SetMaxBeRetransmit;
    UInt8 dummy; //ensure none empty union definition
} gpTest_Server_Input_union_t;

typedef union {
    gpTest_GetCollisionAvoidanceModeInUse_Output_marshall_struct_t gpTest_GetCollisionAvoidanceModeInUse;
    gpTest_GetPacketInPacketMode_Output_marshall_struct_t gpTest_GetPacketInPacketMode;
    gpTest_GetAntennaDiversity_Output_marshall_struct_t gpTest_GetAntennaDiversity;
    gpTest_SetAntenna_Output_marshall_struct_t gpTest_SetAntenna;
    gpTest_GetStatistics_Output_marshall_struct_t gpTest_GetStatistics;
    gpTest_GetSettings_Output_marshall_struct_t gpTest_GetSettings;
    gpTest_SetChannel_Output_marshall_struct_t gpTest_SetChannel;
    gpTest_GetChannel_Output_marshall_struct_t gpTest_GetChannel;
    gpTest_GetContinuousWaveMode_Output_marshall_struct_t gpTest_GetContinuousWaveMode;
    gpTest_GetContTx_Output_marshall_struct_t gpTest_GetContTx;
    gpTest_GetAverageLQI_Output_marshall_struct_t gpTest_GetAverageLQI;
    gpTest_GetAverageRSSI_Output_marshall_struct_t gpTest_GetAverageRSSI;
    gpTest_GetMaxBE_Output_marshall_struct_t gpTest_GetMaxBE;
    gpTest_GetMinBE_Output_marshall_struct_t gpTest_GetMinBE;
    gpTest_GetMaxCSMABackoffs_Output_marshall_struct_t gpTest_GetMaxCSMABackoffs;
    gpTest_GetNumberOfRetries_Output_marshall_struct_t gpTest_GetNumberOfRetries;
    gpTest_GetPanId_Output_marshall_struct_t gpTest_GetPanId;
    gpTest_GetPromiscuousMode_Output_marshall_struct_t gpTest_GetPromiscuousMode;
    gpTest_GetRxAntenna_Output_marshall_struct_t gpTest_GetRxAntenna;
    gpTest_GetRxState_Output_marshall_struct_t gpTest_GetRxState;
    gpTest_GetShortAddress_Output_marshall_struct_t gpTest_GetShortAddress;
    gpTest_GetTxPower_Output_marshall_struct_t gpTest_GetTxPower;
    gpTest_GetExtendedAddress_Output_marshall_struct_t gpTest_GetExtendedAddress;
    gpTest_GetAddressRecognition_Output_marshall_struct_t gpTest_GetAddressRecognition;
    gpTest_GetTxAntenna_Output_marshall_struct_t gpTest_GetTxAntenna;
    gpTest_GetVersionInfo_Output_marshall_struct_t gpTest_GetVersionInfo;
    gpTest_ReadReg_Output_marshall_struct_t gpTest_ReadReg;
    gpTest_WriteReg_Output_marshall_struct_t gpTest_WriteReg;
    gpTest_GetSleepMode_Output_marshall_struct_t gpTest_GetSleepMode;
    gpTest_IsAwake_Output_marshall_struct_t gpTest_IsAwake;
    gpTest_GetLastUsedTxPower_Output_marshall_struct_t gpTest_GetLastUsedTxPower;
    gpTest_GetPhyMode_Output_marshall_struct_t gpTest_GetPhyMode;
    gpTest_CheckDCDCEnable_Output_marshall_struct_t gpTest_CheckDCDCEnable;
#if defined(GP_COMP_GPTEST_BLE)
    gpTest_BleTransmitterTest_Output_marshall_struct_t gpTest_BleTransmitterTest;
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
    gpTest_BleReceiverTest_Output_marshall_struct_t gpTest_BleReceiverTest;
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
    gpTest_BleTestEnd_Output_marshall_struct_t gpTest_BleTestEnd;
#endif /* defined(GP_COMP_GPTEST_BLE) */
    gpTest_SetMcuClockSpeed_Output_marshall_struct_t gpTest_SetMcuClockSpeed;
    gpTest_IpcRestart_Output_marshall_struct_t gpTest_IpcRestart;
    gpTest_GetMeasuredSleepClockFrequency_Output_marshall_struct_t gpTest_GetMeasuredSleepClockFrequency;
    gpTest_GetChipId_Output_marshall_struct_t gpTest_GetChipId;
    gpTest_GetChipVersion_Output_marshall_struct_t gpTest_GetChipVersion;
#if defined(GP_COMP_GPTEST_BLE)
    gpTest_BleGetDeviceAddress_Output_marshall_struct_t gpTest_BleGetDeviceAddress;
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
    gpTest_BleGetNumberOfRxPackets_Output_marshall_struct_t gpTest_BleGetNumberOfRxPackets;
#endif /* defined(GP_COMP_GPTEST_BLE) */
    gpTest_StatisticsCountersGet_Output_marshall_struct_t gpTest_StatisticsCountersGet;
    gpTest_GetPwrCtrlInByPassMode_Output_marshall_struct_t gpTest_GetPwrCtrlInByPassMode;
    gpTest_ReadProductId_Output_marshall_struct_t gpTest_ReadProductId;
    gpTest_SetRxModeOptions_Output_marshall_struct_t gpTest_SetRxModeOptions;
    gpTest_SetChannelForOtherStacks_Output_marshall_struct_t gpTest_SetChannelForOtherStacks;
    gpTest_GetRetransmitOnCcaFail_Output_marshall_struct_t gpTest_GetRetransmitOnCcaFail;
    gpTest_GetRetransmitRandomBackoff_Output_marshall_struct_t gpTest_GetRetransmitRandomBackoff;
    gpTest_GetMinBeRetransmit_Output_marshall_struct_t gpTest_GetMinBeRetransmit;
    gpTest_GetMaxBeRetransmit_Output_marshall_struct_t gpTest_GetMaxBeRetransmit;
    UInt8 dummy; //ensure none empty union definition
} gpTest_Server_Output_union_t;

typedef union {
    gpTest_cbDataConfirm_Input_marshall_struct_t gpTest_cbDataConfirm;
    gpTest_cbDataIndication_Input_marshall_struct_t gpTest_cbDataIndication;
    gpTest_cbEDConfirm_Input_marshall_struct_t gpTest_cbEDConfirm;
#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
    gpTest_cbIpcRestartConfirm_Input_marshall_struct_t gpTest_cbIpcRestartConfirm;
#endif /* defined(GP_HAL_DIVERSITY_INCLUDE_IPC) */
    UInt8 dummy; //ensure none empty union definition
} gpTest_Client_Input_union_t;

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Alias/enum copy macro's
#define gpTest_VersioningIndex_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_VersioningIndex_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_VersioningIndex_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_VersioningIndex_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_ContinuousWaveMode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_ContinuousWaveMode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_ContinuousWaveMode_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_ContinuousWaveMode_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_TxPower_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_TxPower_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_TxPower_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_TxPower_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_SleepMode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_SleepMode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_SleepMode_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_SleepMode_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_AntennaSelection_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_AntennaSelection_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_AntennaSelection_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_AntennaSelection_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_SourceIdentifier_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_SourceIdentifier_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_SourceIdentifier_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_SourceIdentifier_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_CollisionAvoidanceMode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_CollisionAvoidanceMode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_CollisionAvoidanceMode_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_CollisionAvoidanceMode_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_Result_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_Result_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_Result_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_Result_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_SleepClockMeasurementStatus_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_SleepClockMeasurementStatus_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_SleepClockMeasurementStatus_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_SleepClockMeasurementStatus_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_PhyMode_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_PhyMode_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_PhyMode_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_PhyMode_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_BleTxPhy_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_BleTxPhy_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_BleTxPhy_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_BleTxPhy_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)
#define gpTest_BleRxPhy_t_buf2api(pDest, pSource, length, pIndex) UInt8_buf2api(pDest, pSource, length, pIndex)
#define gpTest_BleRxPhy_t_api2buf(pDest, pSource, length, pIndex) UInt8_api2buf(pDest, pSource, length, pIndex)
#define gpTest_BleRxPhy_t_buf2api_1(pDest, pSource, pIndex)       UInt8_buf2api_1(pDest, pSource, pIndex)
#define gpTest_BleRxPhy_t_api2buf_1(pDest, pSource, pIndex)       UInt8_api2buf_1(pDest, pSource, pIndex)

// Structure copy functions
gpMarshall_AckStatus_t gpTest_Statistics_t_buf2api(gpTest_Statistics_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpTest_Statistics_t_api2buf(UInt8Buffer* pDest , const gpTest_Statistics_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_Settings_t_buf2api(gpTest_Settings_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpTest_Settings_t_api2buf(UInt8Buffer* pDest , const gpTest_Settings_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_CntPrio_t_buf2api(gpTest_CntPrio_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpTest_CntPrio_t_api2buf(UInt8Buffer* pDest , const gpTest_CntPrio_t* pSource , UInt16 length , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_StatisticsCounter_t_buf2api(gpTest_StatisticsCounter_t* pDest , UInt8Buffer* pSource , UInt16 length , UInt16* pIndex );
void gpTest_StatisticsCounter_t_api2buf(UInt8Buffer* pDest , const gpTest_StatisticsCounter_t* pSource , UInt16 length , UInt16* pIndex);
// Server functions
gpMarshall_AckStatus_t gpTest_TxPacket_Input_buf2api(gpTest_TxPacket_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_TxPollPacket_Input_buf2api(gpTest_TxPollPacket_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetRxResponsePacket_Input_buf2api(gpTest_SetRxResponsePacket_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_EDScan_Input_buf2api(gpTest_EDScan_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_ExtendedEDScan_Input_buf2api(gpTest_ExtendedEDScan_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetCollisionAvoidanceModeToUse_Input_buf2api(gpTest_SetCollisionAvoidanceModeToUse_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetCollisionAvoidanceModeInUse_Output_api2buf(UInt8Buffer* pDest , gpTest_GetCollisionAvoidanceModeInUse_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetPacketInPacketMode_Input_buf2api(gpTest_SetPacketInPacketMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetPacketInPacketMode_Output_api2buf(UInt8Buffer* pDest , gpTest_GetPacketInPacketMode_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetAntennaDiversity_Input_buf2api(gpTest_SetAntennaDiversity_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetAntennaDiversity_Output_api2buf(UInt8Buffer* pDest , gpTest_GetAntennaDiversity_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetAntenna_Input_buf2api(gpTest_SetAntenna_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetAntenna_Output_api2buf(UInt8Buffer* pDest , gpTest_SetAntenna_Output_marshall_struct_t* pSourceoutput , gpTest_SetAntenna_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpTest_GetStatistics_Output_api2buf(UInt8Buffer* pDest , gpTest_GetStatistics_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_GetSettings_Output_api2buf(UInt8Buffer* pDest , gpTest_GetSettings_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetChannel_Input_buf2api(gpTest_SetChannel_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetChannel_Output_api2buf(UInt8Buffer* pDest , gpTest_SetChannel_Output_marshall_struct_t* pSourceoutput , gpTest_SetChannel_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
void gpTest_GetChannel_Output_api2buf(UInt8Buffer* pDest , gpTest_GetChannel_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetContinuousWaveMode_Input_buf2api(gpTest_SetContinuousWaveMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetContinuousWaveMode_Output_api2buf(UInt8Buffer* pDest , gpTest_GetContinuousWaveMode_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_GetContTx_Output_api2buf(UInt8Buffer* pDest , gpTest_GetContTx_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_GetAverageLQI_Output_api2buf(UInt8Buffer* pDest , gpTest_GetAverageLQI_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_GetAverageRSSI_Output_api2buf(UInt8Buffer* pDest , gpTest_GetAverageRSSI_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetMaxBE_Input_buf2api(gpTest_SetMaxBE_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetMaxBE_Output_api2buf(UInt8Buffer* pDest , gpTest_GetMaxBE_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetMinBE_Input_buf2api(gpTest_SetMinBE_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetMinBE_Output_api2buf(UInt8Buffer* pDest , gpTest_GetMinBE_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetMaxCSMABackoffs_Input_buf2api(gpTest_SetMaxCSMABackoffs_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetMaxCSMABackoffs_Output_api2buf(UInt8Buffer* pDest , gpTest_GetMaxCSMABackoffs_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetNumberOfRetries_Input_buf2api(gpTest_SetNumberOfRetries_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetNumberOfRetries_Output_api2buf(UInt8Buffer* pDest , gpTest_GetNumberOfRetries_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetPanId_Input_buf2api(gpTest_SetPanId_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_GetPanId_Input_buf2api(gpTest_GetPanId_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetPanId_Output_api2buf(UInt8Buffer* pDest , gpTest_GetPanId_Output_marshall_struct_t* pSourceoutput , gpTest_GetPanId_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetPromiscuousMode_Input_buf2api(gpTest_SetPromiscuousMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetPromiscuousMode_Output_api2buf(UInt8Buffer* pDest , gpTest_GetPromiscuousMode_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_GetRxAntenna_Output_api2buf(UInt8Buffer* pDest , gpTest_GetRxAntenna_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetRxState_Input_buf2api(gpTest_SetRxState_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetRxState_Output_api2buf(UInt8Buffer* pDest , gpTest_GetRxState_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetShortAddress_Input_buf2api(gpTest_SetShortAddress_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_GetShortAddress_Input_buf2api(gpTest_GetShortAddress_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetShortAddress_Output_api2buf(UInt8Buffer* pDest , gpTest_GetShortAddress_Output_marshall_struct_t* pSourceoutput , gpTest_GetShortAddress_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetTxPower_Input_buf2api(gpTest_SetTxPower_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetTxPower_Output_api2buf(UInt8Buffer* pDest , gpTest_GetTxPower_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetExtendedAddress_Input_buf2api(gpTest_SetExtendedAddress_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetExtendedAddress_Output_api2buf(UInt8Buffer* pDest , gpTest_GetExtendedAddress_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetAddressRecognition_Input_buf2api(gpTest_SetAddressRecognition_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetAddressRecognition_Output_api2buf(UInt8Buffer* pDest , gpTest_GetAddressRecognition_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_GetTxAntenna_Output_api2buf(UInt8Buffer* pDest , gpTest_GetTxAntenna_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_GetVersionInfo_Input_buf2api(gpTest_GetVersionInfo_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetVersionInfo_Output_api2buf(UInt8Buffer* pDest , gpTest_GetVersionInfo_Output_marshall_struct_t* pSourceoutput , gpTest_GetVersionInfo_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_ReadReg_Input_buf2api(gpTest_ReadReg_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_ReadReg_Output_api2buf(UInt8Buffer* pDest , gpTest_ReadReg_Output_marshall_struct_t* pSourceoutput , gpTest_ReadReg_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_WriteReg_Input_buf2api(gpTest_WriteReg_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_WriteReg_Output_api2buf(UInt8Buffer* pDest , gpTest_WriteReg_Output_marshall_struct_t* pSourceoutput , gpTest_WriteReg_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_PrintfEnable_Input_buf2api(gpTest_PrintfEnable_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetSleepMode_Input_buf2api(gpTest_SetSleepMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetSleepMode_Output_api2buf(UInt8Buffer* pDest , gpTest_GetSleepMode_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_IsAwake_Output_api2buf(UInt8Buffer* pDest , gpTest_IsAwake_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_EnableExternalLna_Input_buf2api(gpTest_EnableExternalLna_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetLastUsedTxPower_Output_api2buf(UInt8Buffer* pDest , gpTest_GetLastUsedTxPower_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetPhyMode_Input_buf2api(gpTest_SetPhyMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetPhyMode_Output_api2buf(UInt8Buffer* pDest , gpTest_GetPhyMode_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_CheckDCDCEnable_Output_api2buf(UInt8Buffer* pDest , gpTest_CheckDCDCEnable_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
#if defined(GP_COMP_GPTEST_BLE)
gpMarshall_AckStatus_t gpTest_BleTransmitterTest_Input_buf2api(gpTest_BleTransmitterTest_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_BleTransmitterTest_Output_api2buf(UInt8Buffer* pDest , gpTest_BleTransmitterTest_Output_marshall_struct_t* pSourceoutput , gpTest_BleTransmitterTest_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleReceiverTest_Output_api2buf(UInt8Buffer* pDest , gpTest_BleReceiverTest_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleTestEnd_Output_api2buf(UInt8Buffer* pDest , gpTest_BleTestEnd_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
gpMarshall_AckStatus_t gpTest_SetModulation_Input_buf2api(gpTest_SetModulation_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
gpMarshall_AckStatus_t gpTest_SetMcuClockSpeed_Input_buf2api(gpTest_SetMcuClockSpeed_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetMcuClockSpeed_Output_api2buf(UInt8Buffer* pDest , gpTest_SetMcuClockSpeed_Output_marshall_struct_t* pSourceoutput , gpTest_SetMcuClockSpeed_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_IpcRestart_Input_buf2api(gpTest_IpcRestart_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_IpcRestart_Output_api2buf(UInt8Buffer* pDest , gpTest_IpcRestart_Output_marshall_struct_t* pSourceoutput , gpTest_IpcRestart_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_GetMeasuredSleepClockFrequency_Input_buf2api(gpTest_GetMeasuredSleepClockFrequency_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetMeasuredSleepClockFrequency_Output_api2buf(UInt8Buffer* pDest , gpTest_GetMeasuredSleepClockFrequency_Output_marshall_struct_t* pSourceoutput , gpTest_GetMeasuredSleepClockFrequency_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_TxCorruptedPacket_Input_buf2api(gpTest_TxCorruptedPacket_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetChipId_Output_api2buf(UInt8Buffer* pDest , gpTest_GetChipId_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_GetChipVersion_Output_api2buf(UInt8Buffer* pDest , gpTest_GetChipVersion_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleGetDeviceAddress_Output_api2buf(UInt8Buffer* pDest , gpTest_BleGetDeviceAddress_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleGetNumberOfRxPackets_Output_api2buf(UInt8Buffer* pDest , gpTest_BleGetNumberOfRxPackets_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
gpMarshall_AckStatus_t gpTest_BleSetNumberTxPacketsInTestMode_Input_buf2api(gpTest_BleSetNumberTxPacketsInTestMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
void gpTest_StatisticsCountersGet_Output_api2buf(UInt8Buffer* pDest , gpTest_StatisticsCountersGet_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetSnifferMode_Input_buf2api(gpTest_SetSnifferMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetPwrCtrlInByPassMode_Input_buf2api(gpTest_SetPwrCtrlInByPassMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetPwrCtrlInByPassMode_Output_api2buf(UInt8Buffer* pDest , gpTest_GetPwrCtrlInByPassMode_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_ReadProductId_Output_api2buf(UInt8Buffer* pDest , gpTest_ReadProductId_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode_Input_buf2api(gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetDpiZbBuffering_Input_buf2api(gpTest_SetDpiZbBuffering_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_EnableDpiZb_Input_buf2api(gpTest_EnableDpiZb_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_EnableDtm_Input_buf2api(gpTest_EnableDtm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetRxModeOptions_Input_buf2api(gpTest_SetRxModeOptions_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetRxModeOptions_Output_api2buf(UInt8Buffer* pDest , gpTest_SetRxModeOptions_Output_marshall_struct_t* pSourceoutput , gpTest_SetRxModeOptions_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetChannelForOtherStacks_Input_buf2api(gpTest_SetChannelForOtherStacks_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetChannelForOtherStacks_Output_api2buf(UInt8Buffer* pDest , gpTest_SetChannelForOtherStacks_Output_marshall_struct_t* pSourceoutput , gpTest_SetChannelForOtherStacks_Input_marshall_struct_t* pSourceinput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_MacSetExpectedRx_Input_buf2api(gpTest_MacSetExpectedRx_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetRetransmitOnCcaFail_Input_buf2api(gpTest_SetRetransmitOnCcaFail_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetRetransmitOnCcaFail_Output_api2buf(UInt8Buffer* pDest , gpTest_GetRetransmitOnCcaFail_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetRetransmitRandomBackoff_Input_buf2api(gpTest_SetRetransmitRandomBackoff_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetRetransmitRandomBackoff_Output_api2buf(UInt8Buffer* pDest , gpTest_GetRetransmitRandomBackoff_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetMinBeRetransmit_Input_buf2api(gpTest_SetMinBeRetransmit_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetMinBeRetransmit_Output_api2buf(UInt8Buffer* pDest , gpTest_GetMinBeRetransmit_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_SetMaxBeRetransmit_Input_buf2api(gpTest_SetMaxBeRetransmit_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetMaxBeRetransmit_Output_api2buf(UInt8Buffer* pDest , gpTest_GetMaxBeRetransmit_Output_marshall_struct_t* pSourceoutput , UInt16* pIndex);
void gpTest_cbDataConfirm_Input_par2api(UInt8Buffer* pDest , UInt8 status , UInt16 packetsSentOK , UInt16 packetsSentError , UInt16* pIndex);
void gpTest_cbDataIndication_Input_par2api(UInt8Buffer* pDest , UInt8 length , gpPd_Offset_t dataOffset , gpPd_Handle_t handle , UInt16* pIndex);
void gpTest_cbEDConfirm_Input_par2api(UInt8Buffer* pDest , UInt8 result , UInt16 channelMask , UInt8* pData , Bool Finished , UInt16* pIndex);
#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
void gpTest_cbIpcRestartConfirm_Input_par2api(UInt8Buffer* pDest , UInt8 result , UInt32 stopDurationUs , UInt32 restartDurationUs , UInt16* pIndex);
#endif /* defined(GP_HAL_DIVERSITY_INCLUDE_IPC) */

// Client functions
void gpTest_TxPacket_Input_par2buf(UInt8Buffer* pDest , UInt16 numberOfPackets , UInt16 intervalInMs , UInt8 dataLength , UInt8* pData , UInt8 txOptions , UInt16* pIndex);
void gpTest_TxPollPacket_Input_par2buf(UInt8Buffer* pDest , UInt16 numberOfPackets , UInt16 intervalInMs , UInt8 dataLength , UInt8* pData , Bool randomData , UInt16* pIndex);
void gpTest_SetRxResponsePacket_Input_par2buf(UInt8Buffer* pDest , UInt32 delayUs , UInt8 dataLength , UInt8* pData , Bool randomData , UInt16* pIndex);
void gpTest_EDScan_Input_par2buf(UInt8Buffer* pDest , UInt16 numberOfScans , UInt16 intervalInMs , UInt16 channelMask , UInt16* pIndex);
void gpTest_ExtendedEDScan_Input_par2buf(UInt8Buffer* pDest , UInt16 numberOfScans , UInt16 intervalInMs , UInt16 channelMask , UInt32 duration_us , UInt16* pIndex);
void gpTest_SetCollisionAvoidanceModeToUse_Input_par2buf(UInt8Buffer* pDest , gpTest_CollisionAvoidanceMode_t newMode , UInt16* pIndex);
void gpTest_GetCollisionAvoidanceModeInUse_Output_buf2par(gpTest_CollisionAvoidanceMode_t* currentMode , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetPacketInPacketMode_Input_par2buf(UInt8Buffer* pDest , Bool newPIP , UInt16* pIndex);
void gpTest_GetPacketInPacketMode_Output_buf2par(Bool* currentMode , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetAntennaDiversity_Input_par2buf(UInt8Buffer* pDest , Bool OnOff , UInt16* pIndex);
void gpTest_GetAntennaDiversity_Output_buf2par(Bool* AntennaDiversityOn , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetAntenna_Input_par2buf(UInt8Buffer* pDest , gpTest_AntennaSelection_t antenna , UInt16* pIndex);
void gpTest_SetAntenna_Output_buf2par(gpTest_Result_t* result , gpTest_AntennaSelection_t antenna , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetStatistics_Output_buf2par(gpTest_Statistics_t* pStatistics , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetSettings_Output_buf2par(gpTest_Settings_t* pSettings , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetChannel_Input_par2buf(UInt8Buffer* pDest , UInt8 newChannel , UInt16* pIndex);
void gpTest_SetChannel_Output_buf2par(gpTest_Result_t* result , UInt8 newChannel , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetChannel_Output_buf2par(UInt8* currentChannel , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetContinuousWaveMode_Input_par2buf(UInt8Buffer* pDest , gpTest_ContinuousWaveMode_t newMode , UInt16* pIndex);
void gpTest_GetContinuousWaveMode_Output_buf2par(gpTest_ContinuousWaveMode_t* currentMode , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetContTx_Output_buf2par(Bool* ContTxEnabled , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetAverageLQI_Output_buf2par(UInt8* lqi , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetAverageRSSI_Output_buf2par(Int8* rssi , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetMaxBE_Input_par2buf(UInt8Buffer* pDest , UInt8 maxBE , UInt16* pIndex);
void gpTest_GetMaxBE_Output_buf2par(UInt8* maxBE , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetMinBE_Input_par2buf(UInt8Buffer* pDest , UInt8 minBE , UInt16* pIndex);
void gpTest_GetMinBE_Output_buf2par(UInt8* minBE , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetMaxCSMABackoffs_Input_par2buf(UInt8Buffer* pDest , UInt8 maxBackoffs , UInt16* pIndex);
void gpTest_GetMaxCSMABackoffs_Output_buf2par(UInt8* maxBackoffs , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetNumberOfRetries_Input_par2buf(UInt8Buffer* pDest , UInt8 retries , UInt16* pIndex);
void gpTest_GetNumberOfRetries_Output_buf2par(UInt8* retries , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetPanId_Input_par2buf(UInt8Buffer* pDest , UInt16 panId , gpTest_SourceIdentifier_t srcId , UInt16* pIndex);
void gpTest_GetPanId_Input_par2buf(UInt8Buffer* pDest , gpTest_SourceIdentifier_t srcId , UInt16* pIndex);
void gpTest_GetPanId_Output_buf2par(UInt16* panId , gpTest_SourceIdentifier_t srcId , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetPromiscuousMode_Input_par2buf(UInt8Buffer* pDest , Bool flag , UInt16* pIndex);
void gpTest_GetPromiscuousMode_Output_buf2par(Bool* flag , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetRxAntenna_Output_buf2par(gpTest_AntennaSelection_t* antenna , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetRxState_Input_par2buf(UInt8Buffer* pDest , Bool rxOn , UInt16* pIndex);
void gpTest_GetRxState_Output_buf2par(Bool* rxOn , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetShortAddress_Input_par2buf(UInt8Buffer* pDest , UInt16 shortAddress , gpTest_SourceIdentifier_t srcId , UInt16* pIndex);
void gpTest_GetShortAddress_Input_par2buf(UInt8Buffer* pDest , gpTest_SourceIdentifier_t srcId , UInt16* pIndex);
void gpTest_GetShortAddress_Output_buf2par(UInt16* shortAddress , gpTest_SourceIdentifier_t srcId , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetTxPower_Input_par2buf(UInt8Buffer* pDest , Int8 transmitPower , UInt16* pIndex);
void gpTest_GetTxPower_Output_buf2par(Int8* transmitPower , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetExtendedAddress_Input_par2buf(UInt8Buffer* pDest , MACAddress_t* pExtendedAddress , UInt16* pIndex);
void gpTest_GetExtendedAddress_Output_buf2par(MACAddress_t* pExtendedAddress , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetAddressRecognition_Input_par2buf(UInt8Buffer* pDest , Bool enable , Bool panCoordinator , UInt16* pIndex);
void gpTest_GetAddressRecognition_Output_buf2par(Bool* addrRecognitionFlag , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetTxAntenna_Output_buf2par(gpTest_AntennaSelection_t* antenna , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetVersionInfo_Input_par2buf(UInt8Buffer* pDest , gpTest_VersioningIndex_t versionType , UInt16* pIndex);
void gpTest_GetVersionInfo_Output_buf2par(gpTest_VersioningIndex_t versionType , gpVersion_ReleaseInfo_t* pVersion , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_ReadReg_Input_par2buf(UInt8Buffer* pDest , UInt32 address , UInt16* pIndex);
void gpTest_ReadReg_Output_buf2par(UInt8* readByte , UInt32 address , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_WriteReg_Input_par2buf(UInt8Buffer* pDest , UInt32 address , UInt8 writeByte , UInt16* pIndex);
void gpTest_WriteReg_Output_buf2par(Bool* successStatus , UInt32 address , UInt8 writeByte , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_PrintfEnable_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpTest_SetSleepMode_Input_par2buf(UInt8Buffer* pDest , gpTest_SleepMode_t mode , UInt16* pIndex);
void gpTest_GetSleepMode_Output_buf2par(gpTest_SleepMode_t* mode , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_IsAwake_Output_buf2par(Bool* isAwake , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_EnableExternalLna_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpTest_GetLastUsedTxPower_Output_buf2par(gpTest_TxPower_t* dBm , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetPhyMode_Input_par2buf(UInt8Buffer* pDest , gpTest_PhyMode_t PhyMode , UInt16* pIndex);
void gpTest_GetPhyMode_Output_buf2par(gpTest_PhyMode_t* phyMode , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_CheckDCDCEnable_Output_buf2par(Bool* enabled , UInt8Buffer* pSource , UInt16* pIndex);
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleTransmitterTest_Input_par2buf(UInt8Buffer* pDest , UInt8 length , UInt8 payloadtype , UInt16* pIndex);
void gpTest_BleTransmitterTest_Output_buf2par(gpTest_Result_t* result , UInt8 length , UInt8 payloadtype , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleReceiverTest_Output_buf2par(gpTest_Result_t* result , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleTestEnd_Output_buf2par(UInt16* result , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_SetModulation_Input_par2buf(UInt8Buffer* pDest , gpTest_BleTxPhy_t modulation , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
void gpTest_SetMcuClockSpeed_Input_par2buf(UInt8Buffer* pDest , UInt8 clockspeed , UInt16* pIndex);
void gpTest_SetMcuClockSpeed_Output_buf2par(gpTest_Result_t* result , UInt8 clockspeed , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_IpcRestart_Input_par2buf(UInt8Buffer* pDest , UInt16 numberOfRestarts , UInt32 intervalInUs , UInt32 stopDurationInUs , UInt32 delayUs , UInt8 trigger , UInt16* pIndex);
void gpTest_IpcRestart_Output_buf2par(gpTest_Result_t* result , UInt16 numberOfRestarts , UInt32 intervalInUs , UInt32 stopDurationInUs , UInt32 delayUs , UInt8 trigger , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetMeasuredSleepClockFrequency_Input_par2buf(UInt8Buffer* pDest , gpTest_SleepMode_t mode , UInt16* pIndex);
void gpTest_GetMeasuredSleepClockFrequency_Output_buf2par(gpTest_SleepClockMeasurementStatus_t* status , gpTest_SleepMode_t mode , UInt32* frequencymHz , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_TxCorruptedPacket_Input_par2buf(UInt8Buffer* pDest , UInt16 numberOfPackets , UInt16 intervalInMs , UInt8 dataLength , UInt8* pData , UInt8 txOptions , UInt16* pIndex);
void gpTest_GetChipId_Output_buf2par(UInt8* chipid , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_GetChipVersion_Output_buf2par(UInt8* chipversion , UInt8Buffer* pSource , UInt16* pIndex);
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleGetDeviceAddress_Output_buf2par(BtDeviceAddress_t* btDeviceAddress , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleGetNumberOfRxPackets_Output_buf2par(UInt16* gpTest_NumberOfRxPackets , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
#if defined(GP_COMP_GPTEST_BLE)
void gpTest_BleSetNumberTxPacketsInTestMode_Input_par2buf(UInt8Buffer* pDest , UInt16 number , UInt16* pIndex);
#endif /* defined(GP_COMP_GPTEST_BLE) */
void gpTest_StatisticsCountersGet_Output_buf2par(gpTest_StatisticsCounter_t* pStatisticsCounters , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetSnifferMode_Input_par2buf(UInt8Buffer* pDest , Bool flag , UInt16* pIndex);
void gpTest_SetPwrCtrlInByPassMode_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpTest_GetPwrCtrlInByPassMode_Output_buf2par(Bool* gpTest_PwrCtrlInByPassMode , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_ReadProductId_Output_buf2par(UInt8* productId , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpTest_SetDpiZbBuffering_Input_par2buf(UInt8Buffer* pDest , UInt8 packetsBuffered , UInt16* pIndex);
void gpTest_EnableDpiZb_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpTest_EnableDtm_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpTest_SetRxModeOptions_Input_par2buf(UInt8Buffer* pDest , Bool enableMultiStandard , Bool enableMultiChannel , Bool enableHighSensitivity , UInt16* pIndex);
void gpTest_SetRxModeOptions_Output_buf2par(gpTest_Result_t* result , Bool enableMultiStandard , Bool enableMultiChannel , Bool enableHighSensitivity , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetChannelForOtherStacks_Input_par2buf(UInt8Buffer* pDest , UInt8 stack1_channel , UInt8 stack2_channel , UInt16* pIndex);
void gpTest_SetChannelForOtherStacks_Output_buf2par(gpTest_Result_t* result , UInt8 stack1_channel , UInt8 stack2_channel , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_MacSetExpectedRx_Input_par2buf(UInt8Buffer* pDest , UInt8 dataLength , UInt8* pData , UInt16* pIndex);
void gpTest_SetRetransmitOnCcaFail_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpTest_GetRetransmitOnCcaFail_Output_buf2par(Bool* gpTest_RetransmitOnCcaFail , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetRetransmitRandomBackoff_Input_par2buf(UInt8Buffer* pDest , Bool enable , UInt16* pIndex);
void gpTest_GetRetransmitRandomBackoff_Output_buf2par(Bool* gpTest_RetransmitRandomBackoff , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetMinBeRetransmit_Input_par2buf(UInt8Buffer* pDest , UInt8 minBERetransmit , UInt16* pIndex);
void gpTest_GetMinBeRetransmit_Output_buf2par(UInt8* gpTest_MinBeRetransmit , UInt8Buffer* pSource , UInt16* pIndex);
void gpTest_SetMaxBeRetransmit_Input_par2buf(UInt8Buffer* pDest , UInt8 maxBERetransmit , UInt16* pIndex);
void gpTest_GetMaxBeRetransmit_Output_buf2par(UInt8* gpTest_MaxBeRetransmit , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_cbDataConfirm_Input_buf2api(gpTest_cbDataConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_cbDataIndication_Input_buf2api(gpTest_cbDataIndication_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
gpMarshall_AckStatus_t gpTest_cbEDConfirm_Input_buf2api(gpTest_cbEDConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
gpMarshall_AckStatus_t gpTest_cbIpcRestartConfirm_Input_buf2api(gpTest_cbIpcRestartConfirm_Input_marshall_struct_t* pDest , UInt8Buffer* pSource , UInt16* pIndex);
#endif /* defined(GP_HAL_DIVERSITY_INCLUDE_IPC) */

void gpTest_InitMarshalling(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _GPTEST_MARSHALLING_H_


