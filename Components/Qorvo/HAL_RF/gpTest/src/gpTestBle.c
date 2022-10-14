/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
 *
 * gpTestBle.c
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
//#define GP_LOCAL_LOG

#include "global.h"
#include "hal.h"
#include "gpTestBle.h"

// gpTest_Ble code was moved to Components/Qorvo/BleController.
// You should now add the "gpBleTest" component to your appliation.
#include "gpBleTest.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_TEST

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpTest_BleStart()
{
    gpBleTest_Start();
}

void gpTest_BleStop()
{
    gpBleTest_Stop();
}

UInt8 gpTest_BleSetChannel(UInt8 channel)
{
    gpBleTest_SetChannel(channel);
    return 0;
}

UInt8 gpTest_BleGetChannel(void)
{
    return gpBleTest_GetChannel();
}

void gpTest_BleSetDeviceAddress(BtDeviceAddress_t* address)
{
    gpBleTest_SetDeviceAddress(address);
}

void gpTest_BleGetDeviceAddress(BtDeviceAddress_t* address)
{
    gpBleTest_GetDeviceAddress(address);
}

//GP_WB_WRITE_PBM_FORMAT_R_ANTENNA
//GP_WB_WRITE_PBM_BLE_FORMAT_T_ANTSEL_INT
//GP_WB_WRITE_PBM_BLE_FORMAT_T_ANTSEL_EXT
UInt8 gpTest_BleSetTxPower(Int8 transmitPower)
{
    return gpBleTest_SetTxPower(transmitPower);
}

Int8 gpTest_BleGetTxPower(void)
{
    return gpBleTest_GetTxPower();
}

gpTest_AntennaSelection_t gpTest_BleGetTxAntenna(void)
{
    return gpBleTest_GetTxAntenna();
}
gpTest_AntennaSelection_t gpTest_BleGetRxAntenna(void)
{
    return gpBleTest_GetRxAntenna();
}

/********************************************
 *  Direct Test Mode functions
 ********************************************/

void gpTest_BleSetNumberTxPacketsInTestMode(UInt16 number)
{
    gpBleTest_SetNumberTxPacketsInTestMode(number);
}


gpTest_Result_t gpTest_BleReceiverTest(void)
{
    return gpBleTest_ReceiverTest();
}

void gpTest_BleSetRxState(Bool flag)
{
    gpBleTest_SetRxState(flag);
}

Bool gpTest_BleGetRxState(void)
{
    return gpBleTest_GetRxState();
}

UInt16 gpTest_BleGetNumberOfRxPackets(void)
{
    return gpBleTest_GetNumberOfRxPackets();
}

gpTest_Result_t gpTest_BleTransmitterTest(UInt8 length, UInt8 payloadtype)
{
    return gpBleTest_TransmitterTest(length, payloadtype);
}

void gpTest_BleAbortTestMode(void)
{
    gpBleTest_AbortTestMode();
}

UInt16 gpTest_BleTestEnd(void)
{
    return gpBleTest_TestEnd();
}


void gpTest_BleSetContinuousWaveMode( gpTest_ContinuousWaveMode_t newMode )
{
    gpTest_ContinuousWaveMode_t bleMode;
#ifdef GP_TEST_DIVERSITY_MAP_BLE_CW_MODES
    if (newMode == CW_MODULATED) {
        bleMode = CW_BLE_MODULATED;
    }
    else if (newMode == CW_UNMODULATED) {
        bleMode = CW_BLE_UNMODULATED;
    }
    else
    {
        bleMode = newMode;
    }
#else
    bleMode = newMode;
#endif // GP_TEST_DIVERSITY_MAP_BLE_CW_MODES
    gpBleTest_SetContinuousWaveMode(bleMode);
}

gpTest_ContinuousWaveMode_t gpTest_BleGetContinuousWaveMode(void)
{
    gpTest_ContinuousWaveMode_t bleMode = gpBleTest_GetContinuousWaveMode();
#ifdef GP_TEST_DIVERSITY_MAP_BLE_CW_MODES
    switch (bleMode)
    {
        case CW_BLE_MODULATED:
        case CW_MODULATED :
        {
            return CW_MODULATED;
        }
        case CW_BLE_UNMODULATED:
        case CW_UNMODULATED:
        {
            return CW_UNMODULATED;
        }
        default :
        {
            return bleMode;
        }
    }
#else
    return bleMode;
#endif // GP_TEST_DIVERSITY_MAP_BLE_CW_MODES
}

void gpTest_BleSetModulation(gpTest_BleTxPhy_t modulation)
{
    gpBleTest_SetModulation(modulation);
}

gpTest_Result_t gpTest_BleSetRxPhyMask(UInt8 rxPhyMask)
{
    return gpBleTest_SetRxPhyMask(rxPhyMask);
}
