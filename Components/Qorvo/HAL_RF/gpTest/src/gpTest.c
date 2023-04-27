/*
 * Copyright (c) 2017-2023, Qorvo Inc
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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpBsp.h"
#include "hal.h"

#include "gpSched.h"
#include "gpHal.h"
#include "gpHal_Statistics.h"
#include "gpReset.h"
#include "gpPd.h"
#include "gpHal_MAC_Ext.h"
#include "gpStat.h"

#include "gpTest.h"
#ifdef GP_COMP_GPHAL_MAC
#include "gpTestMac.h"
#endif
#ifdef GP_COMP_GPTEST_BLE
#include "gpTestBle.h"
#endif

#ifdef GP_DIVERSITY_LOG
#include "gpLog.h"
#endif /* GP_DIVERSITY_LOG */

#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
#include "gpHal_kx_Ipc.h"
#endif



#ifdef GP_COMP_RADIO
#include "gpRadio.h"
#endif

// #ifdef GP_HAL_DIVERSITY_TEST
// #include "gpHal_Test.h"
// #endif

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_TEST

#if !defined(GP_COMP_GPTEST_BLE) && !defined(GP_COMP_GPHAL_MAC)
#error "define a GPHAL MAC or GP_COMP_GPTEST_BLE"
#endif

#define SINGLE_ED_MEAS                      8*16 /*us*/
#define PDM_CLOCK_DUTY_CYCLE                0.5  // 50%

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

gpTest_PhyMode_t gpTest_PhyMode = gpTest_PhyModeMac;
gpTest_ContinuousWaveMode_t gpTest_ContinuousWaveMode;

#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
UInt16 gpTest_IpcRestart_numberOfRestarts = 0;
UInt16 gpTest_IpcRestart_intervalInUs     = 0;
UInt16 gpTest_IpcRestart_stopDurationUs   = 0;
UInt16 gpTest_IpcRestart_delayUs          = 0;
#define IPC_RESTART_TRIGGER_IMMEDIATE           0x00
#define IPC_RESTART_TRIGGER_TX                  0x01
#define IPC_RESTART_TRIGGER_INVALID             0x02
UInt8  gpTest_IpcRestart_trigger          = 0;
UInt32 gpTest_IpcStopDurationUs = 0 ;
UInt32 gpTest_IpcRestartDurationUs = 0 ;

gpHal_IpcBackupRestoreFlags_t gpTest_IpcBackupRestoreFlags;
#endif



/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
static void gpTest_IpcRestart_doRestart(void);
static void gpTest_IpcRestart_doNextStop(void);
#endif

#if   defined(GP_DIVERSITY_GPHAL_K8E)
static gpTest_Result_t gpTest_PdmClkOutCfg(UInt8 gpio, Bool state);
#endif

#if  defined(GP_DIVERSITY_GPHAL_K8E)
static gpTest_Result_t gpTest_PdmClkConfigurePll(UInt32 freqHz);
#endif

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

 void gpTest_UnregisterCallbacks(void)
 {
 #ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacUnregisterCallbacks();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}

void gpTest_Start(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacStart();
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        gpTest_BleStart();
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }

    // init hardware
    gpTest_SetAntennaDiversity(true);
    gpTest_SetAntenna(gpHal_AntennaSelection_Auto);
}

void gpTest_Stop(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacStop();
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        gpTest_BleStop();
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

/**
 * @brief Getter method for the chip ID.
 * @return The identifier of the silicon.
*/
UInt8 gpTest_GetChipId()
{
    return gpHal_GetChipId();
}

/**
 * @brief Getter method for the chip version.
 * @return The metal fix version of the chip.
*/
UInt8 gpTest_GetChipVersion(void)
{
    return gpHal_GetChipVersion();
}


gpHal_Result_t gpTest_SetChannel(UInt8 channel)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacSetChannel(channel);
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        return gpTest_BleSetChannel(channel);
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
    return gpHal_ResultInvalidRequest;
}

UInt8 gpTest_GetChannel(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetChannel();
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        return gpTest_BleGetChannel();
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
    return 0xFF;
}

gpHal_Result_t gpTest_SetAntenna(gpHal_AntennaSelection_t antenna)
{
#if defined(GP_COMP_GPHAL_MAC) || defined(GP_COMP_GPTEST_BLE)
    gpTest_SetRxAntenna(antenna);
    gpTest_SetTxAntenna(antenna);
    return gpHal_ResultSuccess;
#else
    GP_ASSERT_DEV_INT(false);
    return gpHal_ResultInvalidRequest;
#endif
}

void gpTest_SetRxAntenna(gpHal_AntennaSelection_t antenna)
{
    /* Set the Rx Antenna for both ZB and BLE */
#ifdef GP_HAL_DIVERSITY_SINGLE_ANTENNA
    antenna = GP_HAL_DIVERSITY_SINGLE_ANTENNA;
#endif
    if(antenna == gpHal_AntennaSelection_Ant0 || antenna == gpHal_AntennaSelection_Ant1)
    {
        gpHal_SetRxAntenna(antenna);
    }
    else if (antenna == gpHal_AntennaSelection_Auto)
    {
       gpHal_SetRxAntenna(gpHal_AntennaSelection_Auto);
    }
    else
    {
        GP_ASSERT_DEV_INT(false);
    }
}

void gpTest_SetTxAntenna(gpHal_AntennaSelection_t antenna)
{
/* assymetric Set functions :
 * we set the Tx antenna for ZB
 * and not for BLE (for BLE the antenna is the same for RX and TX).
 */
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetTxAntenna(antenna);
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        return; /* do nothing */
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

gpHal_AntennaSelection_t gpTest_GetTxAntenna(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetTxAntenna();
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        return gpTest_BleGetTxAntenna();
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
    return 0xFF;
}

gpHal_AntennaSelection_t gpTest_GetRxAntenna(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpHal_GetRxAntenna();
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        return gpTest_BleGetRxAntenna();
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
    return 0xFF;
}

void gpTest_SetModulation(gpTest_BleTxPhy_t modulation)
{
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        gpTest_BleSetModulation(modulation);
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

void gpTest_SetTxPower (Int8 transmitPower)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetTxPower(transmitPower);
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        gpTest_BleSetTxPower(transmitPower);
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

Int8 gpTest_GetTxPower(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetTxPower();
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        return gpTest_BleGetTxPower();
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
    return -1;
}

gpHal_TxPower_t gpTest_GetLastUsedTxPower(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetLastUsedTxPower();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }

    return 0xFF;
}

void gpTest_SetAntennaDiversity(Bool OnOff)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetAntennaDiversity(OnOff);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}


Bool gpTest_GetAntennaDiversity(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetAntennaDiversity();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }

    return 0xFF;
}


void gpTest_SetContinuousWaveMode(gpTest_ContinuousWaveMode_t newMode)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetContinuousWaveMode(newMode);
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        gpTest_BleSetContinuousWaveMode(newMode);
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

gpTest_ContinuousWaveMode_t gpTest_GetContinuousWaveMode(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetContinuousWaveMode();
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        return gpTest_BleGetContinuousWaveMode();
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }

    return 0xFF;
}

Bool gpTest_GetContTx(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetContTx();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }

    return 0xFF;
}

void gpTest_SetMaxBE(UInt8 maxBE)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetMaxBE(maxBE);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}

UInt8 gpTest_GetMaxBE(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetMaxBE();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }

    return 0xFF;
}

void gpTest_SetMinBE(UInt8 minBE)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetMinBE(minBE);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}

UInt8 gpTest_GetMinBE(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetMinBE();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }

    return 0xFF;
}

void gpTest_SetRxState(Bool flag)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetRxState(flag);
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        gpTest_BleSetRxState(flag);
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

Bool gpTest_GetRxState(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetRxState();
    }
    else
#endif
#ifdef GP_COMP_GPTEST_BLE
    if(gpTest_PhyMode == gpTest_PhyModeBle)
    {
        return gpTest_BleGetRxState();
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }

    return 0xFF;
}

void gpTest_SetNumberOfRetries( UInt8 retries )
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetNumberOfRetries(retries);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}

UInt8 gpTest_GetNumberOfRetries( void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetNumberOfRetries();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
    return 0xFF;
}

void gpTest_SetMaxCSMABackoffs(UInt8 maxBackoffs)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetMaxCSMABackoffs(maxBackoffs);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}

UInt8 gpTest_GetMaxCSMABackoffs(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetMaxCSMABackoffs();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }

    return 0xFF;
}

void gpTest_SetRetransmitOnCcaFail(Bool enable)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetRetransmitOnCcaFail(enable);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}
Bool gpTest_GetRetransmitOnCcaFail(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetRetransmitOnCcaFail();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
    return false;
}

void gpTest_SetRetransmitRandomBackoff(Bool enable)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetRetransmitRandomBackoff(enable);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}
Bool gpTest_GetRetransmitRandomBackoff(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetRetransmitRandomBackoff();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
    return false;
}



void gpTest_SetMinBeRetransmit(UInt8 minBERetransmit)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetMinBeRetransmit(minBERetransmit);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}
UInt8 gpTest_GetMinBeRetransmit(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetMinBeRetransmit();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
    return false;

}
void gpTest_SetMaxBeRetransmit(UInt8 maxBERetransmit)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetMaxBeRetransmit(maxBERetransmit);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}
UInt8 gpTest_GetMaxBeRetransmit(void)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        return gpTest_MacGetMaxBeRetransmit();
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
    return false;

}



void gpTest_SetAsleep(void)
{
      // goto sleep asap
#ifdef GP_SCHED_DIVERSITY_SLEEP
    hal_SleepSetGotoSleepThreshold(1000);
#endif
#ifdef GP_DIVERSITY_LOG
    gpLog_Flush();
#endif
    gpHal_GoToSleepWhenIdle(true);
#ifdef GP_SCHED_DIVERSITY_SLEEP
    hal_SleepSetGotoSleepEnable(true);
#endif
}

Bool gpTest_IsAwake(void)
{
    return gpHal_IsRadioAccessible();
}

void gpTest_SetSleepMode(gpHal_SleepMode_t mode)
{
    gpHal_SetSleepMode(mode);
}

gpHal_SleepMode_t gpTest_GetSleepMode(void)
{
   return gpHal_GetSleepMode();
}

gpHal_SleepClockMeasurementStatus_t gpTest_GetMeasuredSleepClockFrequency(gpHal_SleepMode_t mode, UInt32* frequencymHz)
{
    return gpHal_GetMeasuredSleepClockFrequency(mode, frequencymHz);
}

void gpTest_TxPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacTxPacket(numberOfPackets, intervalInMs, dataLength, pData, txOptions);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}

void gpTest_MacSetExpectedRx(UInt8 dataLength, UInt8* pData)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTestMac_SetExpectedRx(dataLength, pData);
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

void gpTest_TxCorruptedPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacTxCorruptedPacket(numberOfPackets, intervalInMs, dataLength, pData, txOptions);
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

void gpTest_TxPollPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacTxPollPacket(numberOfPackets, intervalInMs, dataLength, pData, txOptions);
#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
        if (gpTest_IpcRestart_trigger == IPC_RESTART_TRIGGER_TX)
        {
            gpSched_ScheduleEvent( gpTest_IpcRestart_delayUs, gpTest_IpcRestart_doNextStop);
            gpTest_IpcRestart_trigger = 0;
        }
#endif // defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
    }
    else
#endif
    {
        GP_ASSERT_DEV_INT(false);
    }
}

void gpTest_SetRxResponsePacket(UInt32 delayUs, UInt8 dataLength, UInt8* pData, UInt8 txOptions)
{
#ifdef GP_COMP_GPHAL_MAC
    if(gpTest_PhyMode == gpTest_PhyModeMac)
    {
        gpTest_MacSetRxResponsePacket(delayUs, dataLength, pData, txOptions);
    }
    else
#endif
    if(false)
    {
        GP_ASSERT_DEV_INT(false);
    }
}

void gpTest_WakeUpGP(void)
{
    if (!gpTest_IsAwake())
    {
        gpHal_GoToSleepWhenIdle(false);
    }
}

void gpTest_EDScan(UInt16 numberOfScans, UInt16 intervalInMs, UInt16 channelMask)
{
#ifdef GP_COMP_GPHAL_MAC
    gpTest_MacEDScan(numberOfScans, intervalInMs, channelMask, SINGLE_ED_MEAS);
#else
    GP_ASSERT_DEV_INT(false);
#endif
}

void gpTest_ExtendedEDScan(UInt16 numberOfScans, UInt16 intervalInMs, UInt16 channelMask, UInt32 duration_us)
{
#ifdef GP_COMP_GPHAL_MAC
    gpTest_MacEDScan(numberOfScans, intervalInMs, channelMask, duration_us);
#else
    GP_ASSERT_DEV_INT(false);
#endif
}

void gpTest_ResetRequest(void)
{
    gpReset_ResetSystem();
}


void gpTest_SetCollisionAvoidanceModeToUse(gpHal_CollisionAvoidanceMode_t newMode)
{
#ifdef GP_COMP_GPHAL_MAC
    gpTest_MacSetCollisionAvoidanceModeToUse(newMode);
#endif
}


gpHal_CollisionAvoidanceMode_t gpTest_GetCollisionAvoidanceModeInUse(void)
{
#ifdef GP_COMP_GPHAL_MAC
    return gpTest_MacGetCollisionAvoidanceModeInUse();
#else
    return 0;
#endif
}


UInt8 gpTest_GetAverageLQI(void)
{
#ifdef GP_COMP_GPHAL_MAC
    return gpTest_MacGetAverageLQI();
#else
    return 0;
#endif
}


Int8 gpTest_GetAverageRSSI(void)
{
#ifdef GP_COMP_GPHAL_MAC
    return gpTest_MacGetAverageRSSI();
#else
    return 0;
#endif
}

void gpTest_GetVersionInfo(gpTest_VersioningIndex_t versionType, gpVersion_ReleaseInfo_t * pVersion)
{
    switch(versionType)
    {
        case gpTest_VersioningIndexChipId:
        {
            UInt16 HWversion;

            HWversion = gpHal_GetHWVersionId();
            pVersion->major  = (HWversion >> 0)  & (BM(6)-1);
            pVersion->minor = 0;
            pVersion->revision = 0;
            pVersion->patch = 0;
            break;
        }
        default:
        {
            pVersion->major = 0xFF;
            pVersion->minor = 0xFF;
            pVersion->revision = 0xFF;
            pVersion->patch = 0xFF;
            break;
        }
    }
}


UInt8 gpTest_ReadReg(UInt32 address)
{
    return gpHal_ReadReg(address);
}

Bool gpTest_WriteReg(UInt32 address, UInt8 writeByte)
{
    gpHal_WriteReg(address, writeByte);

    return true;
}


void gpTest_SetPacketInPacketMode(Bool newPIP)
{
#ifdef GP_COMP_GPHAL_MAC
    gpHal_SetPipMode(newPIP);
#endif
}

Bool gpTest_GetPacketInPacketMode(void)
{
    return gpHal_GetPipMode();
}

void gpTest_GetSettings(gpTest_Settings_t * Settings)
{
#ifdef GP_COMP_GPHAL_MAC
    gpTest_MacGetSettings(Settings);
#endif
}


void gpTest_GetStatistics(gpTest_Statistics_t * Statistics)
{
#ifdef GP_COMP_GPHAL_MAC
    gpTest_MacGetStatistics(Statistics);
#endif
}


void gpTest_ResetStatistics(void)
{
#ifdef GP_COMP_GPHAL_MAC
    gpTest_MacResetStatistics();
#endif

}

void gpTest_PrintfEnable(Bool enable)
{
#ifdef GP_DIVERSITY_LOG
    gpLog_PrintfEnable(enable);
#endif /* GP_DIVERSITY_LOG */
}

gpTest_Result_t gpTest_SetChannelForOtherStacks(UInt8 stack1_channel, UInt8 stack2_channel)
{
    gpTest_Result_t res = gpHal_ResultSuccess;
#ifdef GP_COMP_GPHAL_MAC
    res = gpTest_MacSetChannelForStack(1, stack1_channel);
    if(res == gpHal_ResultSuccess)
    {
        res = gpTest_MacSetChannelForStack(2, stack2_channel);
    }
#endif
    return res;
}

void gpTest_EnableExternalLna(Bool enable)
{
    GP_ASSERT_DEV_INT(false);
    //Need implementation for other chip variants, for older k5,k4 do we want to play with ackTransmitPower for enable/disable Lna ??
}

void gpTest_SetPhyMode(gpTest_PhyMode_t mode)
{
    gpTest_PhyMode = mode;
}

gpTest_PhyMode_t gpTest_GetPhyMode(void)
{
    return gpTest_PhyMode;
}

void gpTest_SetExtendedAddress(MACAddress_t* pExtendedAddress)
{
#ifdef GP_COMP_GPHAL_MAC
    gpHal_SetExtendedAddress(pExtendedAddress, gpHal_SourceIdentifier_0);
#else
    GP_ASSERT_DEV_INT(false);
#endif
}

void gpTest_GetExtendedAddress(MACAddress_t* pExtendedAddress)
{
#ifdef GP_COMP_GPHAL_MAC
    gpHal_GetExtendedAddress(pExtendedAddress, gpHal_SourceIdentifier_0);
#else
    GP_ASSERT_DEV_INT(false);
#endif
}

Bool gpTest_CheckDCDCEnable(void)
{
    return false;
}

void gpTest_SetPanId(UInt16 panID, gpHal_SourceIdentifier_t srcId)
{
#ifdef GP_COMP_GPHAL_MAC
    gpHal_SetPanId(panID, srcId);
#else
    NOT_USED(panID);
    NOT_USED(srcId);
#endif

}
void gpTest_SetPromiscuousMode(Bool flag)
{
#ifdef GP_COMP_GPHAL_MAC
    gpHal_SetPromiscuousMode(flag);
#else
    NOT_USED(flag);
#endif

}
void gpTest_SetSnifferMode(Bool flag)
{
    NOT_USED(flag);
}
void gpTest_SetShortAddress(UInt16 shortAddress, gpHal_SourceIdentifier_t srcId)
{
#ifdef GP_COMP_GPHAL_MAC
    gpHal_SetShortAddress(shortAddress, srcId);
#else
    NOT_USED(shortAddress);
    NOT_USED(srcId);
#endif
}

void gpTest_SetAddressRecognition(Bool enable, Bool panCoordinator)
{
#ifdef GP_COMP_GPHAL_MAC
    gpHal_SetAddressRecognition(enable, panCoordinator);
#else
    NOT_USED(enable);
    NOT_USED(panCoordinator);
#endif
}

UInt16 gpTest_GetPanId(gpHal_SourceIdentifier_t srcId)
{
#ifdef GP_COMP_GPHAL_MAC
    return gpHal_GetPanId(srcId);
#else
    return 0xFFFF;
#endif
}

Bool gpTest_GetPromiscuousMode(void)
{
#ifdef GP_COMP_GPHAL_MAC
    return gpHal_GetPromiscuousMode();
#else
    return false;
#endif
}

UInt16 gpTest_GetShortAddress(gpHal_SourceIdentifier_t srcId)
{
#ifdef GP_COMP_GPHAL_MAC
    return gpHal_GetShortAddress(srcId);
#else
    return 0xFFFF;
#endif
}
Bool gpTest_GetAddressRecognition(void)
{
#ifdef GP_COMP_GPHAL_MAC
    return gpHal_GetAddressRecognition();
#else
    return false;
#endif
}

gpHal_Result_t gpTest_SetMcuClockSpeed(UInt8 clockSpeed)
{
    if(clockSpeed > 0xa)
    {
        return 0xff;
    }
#ifdef GP_DIVERSITY_CORTEXM4
    HAL_SET_MCU_CLOCK_SPEED(clockSpeed);
    return gpHal_ResultSuccess;
#else
    return 0x08;
#endif // GP_DIVERSITY_CORTEXM4
}

#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
ALWAYS_INLINE UInt32 calcTimeDiff(UInt32 t0, UInt32 t1) // should really be implemented in hal
{
    return t0 < t1 ? (t1 - t0) : ((0xFFFFFFFF-t0) + t1);
}

void gpTest_IpcRestart_doRestart(void)
{
    UInt32 t_start;
    UInt32 t_end;

    HAL_TIMER_GET_CURRENT_TIME_1US(t_start);
    gpHal_IpcRestart(&gpTest_IpcBackupRestoreFlags);
    HAL_TIMER_GET_CURRENT_TIME_1US(t_end);
    gpTest_IpcRestartDurationUs = max(gpTest_IpcRestartDurationUs, calcTimeDiff(t_start,t_end));

    gpTest_IpcRestart_numberOfRestarts--;
    gpSched_ScheduleEvent( gpTest_IpcRestart_intervalInUs, gpTest_IpcRestart_doNextStop );
}

void gpTest_IpcRestart_doNextStop(void)
{
    UInt32 t_start;
    UInt32 t_end;

    if (0 == gpTest_IpcRestart_numberOfRestarts)
    {
        gpTest_cbIpcRestartConfirm(0, gpTest_IpcStopDurationUs, gpTest_IpcRestartDurationUs);
        return;
    }

    HAL_TIMER_GET_CURRENT_TIME_1US(t_start);
    gpHal_IpcStop(&gpTest_IpcBackupRestoreFlags);
    HAL_TIMER_GET_CURRENT_TIME_1US(t_end);
    gpTest_IpcStopDurationUs = max(gpTest_IpcStopDurationUs, calcTimeDiff(t_start,t_end));

    gpSched_ScheduleEvent( gpTest_IpcRestart_stopDurationUs, gpTest_IpcRestart_doRestart);
}
#endif // defined(GP_HAL_DIVERSITY_INCLUDE_IPC)

gpHal_Result_t gpTest_IpcRestart(UInt16 numberOfRestarts, UInt32 intervalInUs, UInt32 stopDurationUs, UInt32 delayUs, UInt8 trigger)
{
#if defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
    if (trigger >= IPC_RESTART_TRIGGER_INVALID)
    {
        return gpHal_ResultInvalidParameter;
    }

    gpTest_IpcRestart_numberOfRestarts = numberOfRestarts;
    gpTest_IpcRestart_intervalInUs     = intervalInUs;
    gpTest_IpcRestart_stopDurationUs   = stopDurationUs;
    gpTest_IpcRestart_trigger          = trigger;
    gpTest_IpcRestart_delayUs          = delayUs;
    gpTest_IpcStopDurationUs           = 0;
    gpTest_IpcRestartDurationUs        = 0;

    if (gpTest_IpcRestart_trigger == IPC_RESTART_TRIGGER_IMMEDIATE)
    {
        gpSched_ScheduleEvent( gpTest_IpcRestart_delayUs, gpTest_IpcRestart_doNextStop);
    }
    return gpHal_ResultSuccess;
#else
    NOT_USED(numberOfRestarts);
    NOT_USED(intervalInUs);
    NOT_USED(stopDurationUs);
    NOT_USED(delayUs);
    NOT_USED(trigger);
    return gpHal_ResultInvalidRequest;
#endif // defined(GP_HAL_DIVERSITY_INCLUDE_IPC)
}


void gpTest_StatisticsCountersGet(gpTest_StatisticsCounter_t* pStatisticsCounters)
{
    NOT_USED(pStatisticsCounters);
}

void gpTest_StatisticsCountersClear(void)
{
}

void gpTest_ReadProductId(UInt8 *productId)
{
    UInt64 productId0 = GP_WB_READ_NVR_PRODUCT_ID_0();
    MEMCPY(productId, &productId0, sizeof(UInt64));
    UInt16 productId1 = GP_WB_READ_NVR_PRODUCT_ID_1();
    MEMCPY(productId+sizeof(UInt64), &productId1, sizeof(UInt16));

}

void gpTest_SetDpiZbBuffering(UInt8 packetsBuffered)
{
    NOT_USED(packetsBuffered);
}

void gpTest_EnableDpiZb(Bool enable)
{
    NOT_USED(enable);
}


void gpTest_SetPwrCtrlInByPassMode(Bool enable)
{
    NOT_USED(enable);
}

Bool gpTest_GetPwrCtrlInByPassMode(void)
{
    return false;
}

void gpTest_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode(Bool enable)
{
    gpHal_SetRxLnaAttDuringTimeoutForRssiBasedAgcMode(enable);
}

void gpTest_EnableDtm(Bool enable)
{
    NOT_USED(enable);
}

gpTest_Result_t gpTest_SetRxModeOptions(Bool enableMultiStandard, Bool enableMultiChannel, Bool enableHighSensitivity)
{
    gpTest_Result_t result = gpHal_ResultSuccess;
#ifdef GP_COMP_RADIO
#ifdef GP_COMP_GPTEST_BLE
    // Multichannel and High sensitivity are only valid for 802.15.4, check that
    if (gpTest_GetPhyMode() == gpTest_PhyModeBle && (enableHighSensitivity || enableMultiChannel))
    {
        return gpHal_ResultInvalidParameter;
    }
    // Antena diversity is only valid for 802.15.4, and can only be enabled while in that mode. This is already controlled in the AD command so no need to add extra checks on that
#endif //def GP_COMP_GPTEST_BLE
    if (gpRadio_SetRxMode(enableMultiStandard, enableMultiChannel, enableHighSensitivity) != gpRadio_StatusSuccess)
    {
        result = gpHal_ResultInvalidParameter;
    }
#endif //def GP_COMP_RADIO
    return result;
}

gpTest_Result_t gpTest_SetPdmClk(gpTest_PDMClkSrc_t src, UInt32 freqHz, UInt8 gpio)
{
#if  defined(GP_DIVERSITY_GPHAL_K8E)
    gpTest_Result_t result = gpHal_ResultSuccess;
    static UInt8 lastGpio = 0xFF;

    // Disable the last GPIO mapping:
    if ((lastGpio != 0xFF) && (lastGpio != gpio))
    {
        gpTest_PdmClkOutCfg(lastGpio, false);
    }

    // Set GPIO mapping:
    result = gpTest_PdmClkOutCfg(gpio, true);

    if (result != gpHal_ResultSuccess)
    {
        return result;
    }

    // Store the last GPIO to disable mapping before switching:
    lastGpio = gpio;

    switch(src)
    {
        case gpTest_PDMClkSrc_None:
        {
            GP_WB_WRITE_ASP_CLK_2M_ENABLE(0);

#if defined(GP_DIVERSITY_GPHAL_K8E)
            GP_WB_WRITE_ASP_USE_FRACT_CLOCK(0);
            GP_WB_WRITE_BBPLL_ENABLE_FRACT_CLOCK(0);
#endif
            // Disable GPIO mapping:
            gpTest_PdmClkOutCfg(gpio, false);
            break;
        }
        case gpTest_PDMClkSrc_2M:
        {
            // For this clock the required frequency should be equal to 2MHz:
            if (freqHz != 2000000)
            {
                result = gpHal_ResultInvalidParameter;
                break;
            }

#if defined(GP_DIVERSITY_GPHAL_K8E)
            GP_WB_WRITE_ASP_USE_FRACT_CLOCK(0);
            GP_WB_WRITE_BBPLL_ENABLE_FRACT_CLOCK(0);
#endif
            GP_WB_WRITE_ASP_CLK_2M_ENABLE(1);
            break;
        }
#if defined(GP_DIVERSITY_GPHAL_K8E)
        case gpTest_PDMClkSrc_PLL:
        {
            result = gpTest_PdmClkConfigurePll(freqHz);

            if (result != gpHal_ResultSuccess)
            {
                break;
            }

            GP_WB_WRITE_ASP_CLK_2M_ENABLE(0);
            GP_WB_WRITE_BBPLL_ENABLE_FRACT_CLOCK(1);
            GP_WB_WRITE_ASP_USE_FRACT_CLOCK(1);
            break;
        }
#endif
        default:
        {
            result = gpHal_ResultInvalidParameter;
        }
    }

    return result;
#else
    return gpHal_ResultUnsupported;
#endif
}

// K8A only:
#if    defined(GP_DIVERSITY_GPHAL_K8E)
static gpTest_Result_t gpTest_PdmClkOutCfg(UInt8 gpio, Bool state)
{
    switch(gpio)
    {
// K8C and K8D and K8E:
        case 3:
        {
            GP_WB_WRITE_IOB_GPIO_3_ALTERNATE(GP_WB_ENUM_GPIO_3_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_3_ALTERNATE_ENABLE(state);
            break;
        }
        case 11:
        {
            GP_WB_WRITE_IOB_GPIO_11_ALTERNATE(GP_WB_ENUM_GPIO_11_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_11_ALTERNATE_ENABLE(state);
            break;
        }
        case 17:
        {

            GP_WB_WRITE_IOB_GPIO_17_ALTERNATE(GP_WB_ENUM_GPIO_17_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_17_ALTERNATE_ENABLE(state);
            break;
        }
// K8C only:

// K8D and K8E:
#if defined(GP_DIVERSITY_GPHAL_K8E)
        case 0:
        {
            GP_WB_WRITE_IOB_GPIO_0_ALTERNATE(GP_WB_ENUM_GPIO_0_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_0_ALTERNATE_ENABLE(state);
            break;
        }
        case 1:
        {
            GP_WB_WRITE_IOB_GPIO_1_ALTERNATE(GP_WB_ENUM_GPIO_1_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_1_ALTERNATE_ENABLE(state);
            break;
        }
        case 2:
        {
            GP_WB_WRITE_IOB_GPIO_2_ALTERNATE(GP_WB_ENUM_GPIO_2_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_2_ALTERNATE_ENABLE(state);
            break;
        }
        case 13:
        {
            GP_WB_WRITE_IOB_GPIO_13_ALTERNATE(GP_WB_ENUM_GPIO_13_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_13_ALTERNATE_ENABLE(state);
            break;
        }
        case 14:
        {
            GP_WB_WRITE_IOB_GPIO_14_ALTERNATE(GP_WB_ENUM_GPIO_14_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_14_ALTERNATE_ENABLE(state);
            break;
        }
        case 15:
        {
            GP_WB_WRITE_IOB_GPIO_15_ALTERNATE(GP_WB_ENUM_GPIO_15_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_15_ALTERNATE_ENABLE(state);
            break;
        }
        case 16:
        {
            GP_WB_WRITE_IOB_GPIO_16_ALTERNATE(GP_WB_ENUM_GPIO_16_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_16_ALTERNATE_ENABLE(state);
            break;
        }
        case 18:
        {
            GP_WB_WRITE_IOB_GPIO_18_ALTERNATE(GP_WB_ENUM_GPIO_18_ALTERNATES_ASP_CLK);
            GP_WB_WRITE_IOB_GPIO_18_ALTERNATE_ENABLE(state);
            break;
        }
#endif
        default:
        {
            return gpHal_ResultInvalidParameter;
        }
    }

    return gpHal_ResultSuccess;
}

static gpTest_Result_t gpTest_PdmClkConfigurePll(UInt32 reqFreqHz)
{
    // The calculations are described in the chapter 10.1 of GP_P008_UM_17700_User_Manual_QPG5072.pdf
    gpTest_Result_t result = gpHal_ResultSuccess;
    Int16 skipCnt = 0;
    UInt16 clkHigh = 0;
    Int16 period = 0;

    // Check if given frequency is in the range:
    if (!GP_TEST_PDM_CLK_FREQ_VALIDATE(reqFreqHz))
    {
        return gpHal_ResultInvalidParameter;
    }

    // If the reference frequency is not integer divisible by the desired frequency, find the best coefficients:
    if (PDM_REF_SRC_FREQ_HZ % reqFreqHz)
    {
        UInt32 lowestDiff = 0xFFFFFFFF;
        UInt16 bestSkipCnt = 0;
        UInt16 bestPeriod = 0;
        UInt32 calcFreqHz = 0;
        Int32 diffHz = 0;

        // The highest "period" value fits the best, so look for it backwards (511 is max period value):
        for (period = 511; period >= 0; period--)
        {
            // Calculate the "clock skip count" coefficient:
            skipCnt = 1.0 / ((((float)PDM_REF_SRC_FREQ_HZ / reqFreqHz) / (period + 1)) - 1);

            // Value should be positive and fits into register (max 511):
            if (skipCnt > 511)
            {
                continue;
            }

            if (skipCnt < 0)
            {
                skipCnt = 0;
            }

            // Calculate the output frequency from the coefficients:
            if (skipCnt > 0)
            {
                calcFreqHz = PDM_REF_SRC_FREQ_HZ / ((period + 1) * ((1.0 / skipCnt) + 1));
            }
            else
            {
                calcFreqHz = PDM_REF_SRC_FREQ_HZ / (period + 1);
            }

            // Calculate the difference between required and calculated frequencies:
            diffHz = reqFreqHz - calcFreqHz;
            diffHz = ABS(diffHz);

            // Store the best coefficients that gives the least error:
            if (diffHz < lowestDiff)
            {
                lowestDiff = diffHz;
                bestPeriod = period;
                bestSkipCnt = skipCnt;

                if (diffHz == 0)
                {
                    break;
                }
            }
        }

        period = bestPeriod;
        skipCnt = bestSkipCnt;
    }
    else
    {
        period = (PDM_REF_SRC_FREQ_HZ / reqFreqHz) - 1;
    }

    // Calculate the clock high coefficient (related to duty cycle):
    clkHigh = ((float)PDM_REF_SRC_FREQ_HZ / reqFreqHz) * PDM_CLOCK_DUTY_CYCLE + 0.5;

    // Configure fractional clock:
#if   defined(GP_DIVERSITY_GPHAL_K8E)
    GP_WB_WRITE_BBPLL_FRACT_CLOCK_CYCLE_SKIP_COUNT(skipCnt);
#endif
    GP_WB_WRITE_BBPLL_FRACT_CLOCK_PERIOD(period);
    GP_WB_WRITE_BBPLL_FRACT_CLOCK_HIGH(clkHigh);

    return result;
}

#endif

void gpTest_EnableAgc(Bool enable)
{
    GP_ASSERT_SYSTEM(false);
    NOT_USED(enable);
}
