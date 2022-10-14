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
#include "gpBsp.h"
#include "hal.h"

#include "gpSched.h"
#include "gpHal.h"
#include "gpReset.h"
#include "gpPd.h"
#include "gpHal_MAC_Ext.h"
#include "gpStat.h"
#include "gpBleTest.h"
#include "gpPoolMem.h"
#include "gpLog.h"


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLETEST

#define GP_TEST_BLE_NR_OF_TX_PACKETS_DEFAULT    0xFFFF

// sync word (4) - pdu header (1) - pdu length(1) - crc (3)
#define TEST_MODE_NR_COMMON_OVERHEAD_BYTES      (4 + 1 + 1 + 3)
// preamble (1) - sync word (4) - pdu header (1) - pdu length(1) - crc (3)
#define TEST_MODE_NR_OF_OVERHEAD_BYTES          (1 + TEST_MODE_NR_COMMON_OVERHEAD_BYTES)
// preamble (2) - sync word (4) - pdu header (1) - pdu length(1) - crc (3)
#define TEST_MODE_NR_OF_OVERHEAD_BYTES_2MB      (2 + TEST_MODE_NR_COMMON_OVERHEAD_BYTES)

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

gpHal_BleCallbacks_t gpBleTest_Callbacks;
typedef struct gpBleTest_Config_s
{
    gpTest_ContinuousWaveMode_t cwMode;
    gpPd_Handle_t               testPdHandle;
    UInt8                       blePhysicalChannel;
    UInt32                      accessCode;
    gpPad_Handle_t              padHandle;
    Bool                        bleRxState;
    gpHal_BleTxPhy_t            txPhy;
    UInt16                      numberOfRxPackets;
    UInt16                      numberOfTxPackets;
    UInt8                       bleRxPhyMask;
}gpBleTest_Config_t;

static gpBleTest_Config_t gpBleTest_Config;
/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
static void Test_PRBS9(UInt8* buf, UInt8 len);
static gpHal_BleRxPhy_t Test_TxPhyToRxPhy(gpHal_BleTxPhy_t txPhy);
static void gpBleTest_BleHalDataInd(UInt8 connId, gpPd_Loh_t pdLoh);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* function to generate a PRBS9 testpattern */
void Test_PRBS9(UInt8* buf, UInt8 len)
{
    UInt16 j;
    UInt16 a = 0x01ff;
    UInt16 newbit;

    for(j = 0; j< len; j++)
    {
        newbit = (((a >> (9-1)) ^ (a >> (9-5))) & 1);
        a = ((a << 1) | newbit) & 0x01ff;

        /* Create byte output of bit stream */
        buf[j] |=a>>(j+1);
        if((j+1)<len)
        {
            buf[j+1] = a >> (8-(j%8));
        }

    }
}

gpHal_BleRxPhy_t Test_TxPhyToRxPhy(gpHal_BleTxPhy_t txPhy)
{
    switch (txPhy)
    {
        case gpHal_BleTxPhy1Mb:
            return gpHal_BleRxPhy1Mb;
        case gpHal_BleTxPhy2Mb:
            return gpHal_BleRxPhy2Mb;
        default:
            return gpHal_BleRxPhyInvalid;
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpBleTest_Init(void)
{
}

void gpBleTest_ResetRequest(void)
{
}

void gpBleTest_Start(void)
{
    gpHal_EnableDataIndInterrupts(1);
    gpHal_EnableDataConfInterrupts(1);

    gpBleTest_Config.blePhysicalChannel = 0xff;
    gpBleTest_Config.txPhy = gpHal_BleTxPhy1Mb;
    gpBleTest_Config.bleRxPhyMask = 0;
    gpBleTest_Config.testPdHandle = GP_PD_INVALID_HANDLE;
    gpBleTest_Config.numberOfTxPackets = GP_TEST_BLE_NR_OF_TX_PACKETS_DEFAULT;
    gpBleTest_Config.bleRxState = false;
    gpBleTest_Config.accessCode = GPHAL_BLE_DIRECT_TEST_MODE_SYNCWORD;
    gpBleTest_Callbacks.cbDataInd = gpBleTest_BleHalDataInd;

    gpBleTest_Config.cwMode = CW_OFF;
#ifndef GP_COMP_UNIT_TEST  // why???
    gpHal_BleRegisterCallbacks(&gpBleTest_Callbacks);

        gpPad_Attributes_t padInitAttributes;
        padInitAttributes.channels[0] = gpBleTest_Config.blePhysicalChannel;
        padInitAttributes.channels[1] = 0xFF;
        padInitAttributes.channels[2] = 0xFF;
#ifdef GP_HAL_DIVERSITY_SINGLE_ANTENNA
        padInitAttributes.antenna = GP_HAL_DIVERSITY_SINGLE_ANTENNA;
#else
        padInitAttributes.antenna = 0;
#endif
        padInitAttributes.txPower = GPHAL_DEFAULT_TRANSMIT_POWER;
        padInitAttributes.minBE   = 3;
        padInitAttributes.maxBE   = 5;
        padInitAttributes.maxCsmaBackoffs = 4;
        padInitAttributes.maxFrameRetries = 3;
        padInitAttributes.csma = gpHal_CollisionAvoidanceModeNoCCA;
        padInitAttributes.cca = gpHal_CCAModeEnergy;

    if (gpPad_CheckPadValid(gpBleTest_Config.padHandle) != gpPad_ResultValidHandle)
    {
        gpBleTest_Config.padHandle = gpPad_GetPad(&padInitAttributes);
    }
    else
    {
        gpPad_SetAttributes(gpBleTest_Config.padHandle, &padInitAttributes); //Re-init old handle
    }

    GP_ASSERT_SYSTEM(gpPad_CheckPadValid(gpBleTest_Config.padHandle) == gpPad_ResultValidHandle);
#endif // #ifdef GP_COMP_UNIT_TEST
}

void gpBleTest_Stop(void)
{
#ifndef GP_COMP_UNIT_TEST
    if (gpPad_CheckPadValid(gpBleTest_Config.padHandle) == gpPad_ResultValidHandle)
    {
        // init hardware (needs to be done before freeing pad)
        //gpTest_SetAntennaDiversity(true);
        //gpTest_SetAntenna(gpHal_AntennaSelection_Auto);

        gpPad_FreePad(gpBleTest_Config.padHandle);
    }
#endif
}

void gpBleTest_SetChannel(UInt8 channel)
{
    gpBleTest_Config.blePhysicalChannel = channel;
    gpHal_BleTestSetChannel(channel);
}

UInt8 gpBleTest_GetChannel(void)
{
    return gpBleTest_Config.blePhysicalChannel;
}

void gpBleTest_SetDeviceAddress(BtDeviceAddress_t* address)
{
    gpHal_BleSetDeviceAddress(address);
}

void gpBleTest_GetDeviceAddress(BtDeviceAddress_t* address)
{
    gpHal_BleGetDeviceAddress(address);
}

//GP_WB_WRITE_PBM_FORMAT_R_ANTENNA
//GP_WB_WRITE_PBM_BLE_FORMAT_T_ANTSEL_INT
//GP_WB_WRITE_PBM_BLE_FORMAT_T_ANTSEL_EXT
UInt8 gpBleTest_SetTxPower(UInt8 transmitPower)
{
    gpPad_SetTxPower(gpBleTest_Config.padHandle, transmitPower);
    return gpHal_BleSetTxPower(transmitPower);
}

Int8 gpBleTest_GetTxPower(void)
{
    Int8 res = gpHal_BleGetTxPower();
    GP_LOG_SYSTEM_PRINTF(" TXPOWER %d ",2, res);
    return res;//gpHal_BleGetTxPower();
}

gpHal_AntennaSelection_t gpBleTest_GetTxAntenna(void)
{
    return gpHal_GetBleAntenna();
}

gpHal_AntennaSelection_t gpBleTest_GetRxAntenna(void)
{
    return gpHal_GetBleAntenna();
}


/********************************************
 *  Direct Test Mode functions
 ********************************************/

void gpBleTest_SetNumberTxPacketsInTestMode(UInt16 number)
{
    gpBleTest_Config.numberOfTxPackets = number;
}


gpHal_Result_t gpBleTest_ReceiverTest(void)
{
    gpHal_TestInfo_t testInfo;
    MEMSET(&testInfo, 0, sizeof(gpHal_TestInfo_t));

    testInfo.tx = false;
    testInfo.channel = gpBleTest_Config.blePhysicalChannel;
    testInfo.accesscode = gpBleTest_Config.accessCode;
    testInfo.antenna = gpHal_GetBleAntenna();
    testInfo.phy.rxPhy = Test_TxPhyToRxPhy(gpBleTest_Config.txPhy);
    testInfo.pdLoh.handle = GP_PD_INVALID_HANDLE;
    testInfo.rxPhyMask = gpBleTest_Config.bleRxPhyMask;
    // set channel and start receiving packets - count the nbr of received packets
    return gpHal_BleStartTestMode(&testInfo);
}

void gpBleTest_SetRxState(Bool flag)
{
    gpHal_Result_t result;

    if (flag == gpBleTest_Config.bleRxState)
    {
        return;
    }

    if (flag)
    {
        gpHal_TestInfo_t testInfo;
        MEMSET(&testInfo, 0, sizeof(gpHal_TestInfo_t));

        testInfo.tx = false;
        testInfo.channel = gpBleTest_Config.blePhysicalChannel;
        testInfo.accesscode = gpBleTest_Config.accessCode;
        testInfo.antenna = gpHal_GetBleAntenna();
        testInfo.phy.rxPhy = Test_TxPhyToRxPhy(gpBleTest_Config.txPhy);
        testInfo.pdLoh.handle = GP_PD_INVALID_HANDLE;
        testInfo.rxPhyMask = gpBleTest_Config.bleRxPhyMask;
        result = gpHal_BleStartTestMode(&testInfo);
        GP_ASSERT_DEV_INT(gpHal_ResultSuccess == result);
    }
    else
    {
        gpHal_BleTestModeEnd_t info;

        gpHal_BleEndTestMode(&info);

        gpBleTest_Config.numberOfRxPackets = info.nrOfPackets;
    }

    gpBleTest_Config.bleRxState = flag;
}

Bool gpBleTest_GetRxState(void)
{
    return gpBleTest_Config.bleRxState;
}

UInt16 gpBleTest_GetNumberOfRxPackets(void)
{
    return gpBleTest_Config.numberOfRxPackets;
}

gpHal_Result_t gpBleTest_TransmitterTest(UInt8 length, UInt8 payloadtype)
{
    gpHal_TestInfo_t testInfo;
    MEMSET(&testInfo, 0, sizeof(gpHal_TestInfo_t));
    UInt8 *testFrame=NULL;
    gpHal_Result_t result;

    result = (gpBleTest_Config.blePhysicalChannel<= 0x27) ? gpHal_ResultSuccess: gpHal_ResultInvalidParameter;

    if (gpBleTest_Config.testPdHandle != GP_PD_INVALID_HANDLE || gpBleTest_Config.numberOfTxPackets == 0)
    {
        return gpHal_ResultInvalidParameter;
    }

    if(result == 0)
    {
        testInfo.tx = true;
        testInfo.accesscode = gpBleTest_Config.accessCode;
        testInfo.channel = gpBleTest_Config.blePhysicalChannel;
        testInfo.antenna = gpHal_GetBleAntenna();
        testInfo.phy.txPhy = gpBleTest_Config.txPhy;
        testInfo.txPacketCount = gpBleTest_Config.numberOfTxPackets;

        testFrame = GP_POOLMEM_MALLOC(length);
        if(testFrame)
        {
            switch(payloadtype)
            {
                case 0: Test_PRBS9 (testFrame, length); break;
                case 1: MEMSET(testFrame, 0x0F, length); break;
                case 2: MEMSET(testFrame, 0x55, length); break;
                case 3: MEMSET(testFrame, 0xFF, length); break; // PRBS15 is only used on the tester side: not needed in IUT
                case 4: MEMSET(testFrame, 0xFF, length); break;
                case 5: MEMSET(testFrame, 0x00, length); break;
                case 6: MEMSET(testFrame, 0xF0, length); break;
                case 7: MEMSET(testFrame, 0xAA, length); break;
                default: break;
            }
        }
        else
        {
            return gpHal_ResultInvalidParameter;
        }

        testInfo.pdLoh.handle = gpPd_GetCustomPd(gpPd_BufferTypeBle, GP_HAL_PBM_MAX_SIZE);

        if(gpPd_CheckPdValid(testInfo.pdLoh.handle) == gpPd_ResultValidHandle)
        {
            testInfo.pdLoh.length=0;
            testInfo.pdLoh.offset= (GP_HAL_PBM_MAX_SIZE - 1);

            // TestPattern
            gpPd_PrependWithUpdate(&testInfo.pdLoh, length, testFrame);
            // length
            gpPd_PrependWithUpdate(&testInfo.pdLoh, 1, &length);
            // header
            gpPd_PrependWithUpdate(&testInfo.pdLoh, 1, &payloadtype);
            gpPd_PrependWithUpdate(&testInfo.pdLoh, 4, (UInt8*)&testInfo.accesscode);

            // set channel and start transmitting packets
            result = gpHal_BleStartTestMode(&testInfo);

            if(gpHal_ResultSuccess != result)
            {
                gpPd_FreePd(testInfo.pdLoh.handle);
            }
            else
            {
                // Do not free the Pd (PBM) yet: the RT subsystem is still using it
                gpBleTest_Config.testPdHandle = testInfo.pdLoh.handle;
            }
        }
        else
        {
            result = 1;
        }
        gpPoolMem_Free(testFrame);
    }
    return result;
}

void gpBleTest_AbortTestMode(void)
{
    if (gpBleTest_Config.testPdHandle != GP_PD_INVALID_HANDLE)
    {
        gpHal_BleTestModeEnd_t testInfo;
        gpHal_BleEndTestMode(&testInfo);
        GP_ASSERT_DEV_INT(testInfo.pdHandle == gpBleTest_Config.testPdHandle);
        gpPd_FreePd(gpBleTest_Config.testPdHandle);
        gpBleTest_Config.testPdHandle = GP_PD_INVALID_HANDLE;
    }
}

UInt16 gpBleTest_TestEnd(void)
{
    gpHal_BleTestModeEnd_t testInfo;

    if(!gpHal_BleTestModeIsActive())
    {
        return 0;
    }
    gpHal_BleEndTestMode(&testInfo);

    if (gpBleTest_Config.testPdHandle != GP_PD_INVALID_HANDLE)
    {
        //Do we need to check the Pd here when using pd ram variant ?
        GP_ASSERT_DEV_INT(testInfo.pdHandle == gpBleTest_Config.testPdHandle);
        gpPd_FreePd(gpBleTest_Config.testPdHandle);
        gpBleTest_Config.testPdHandle = GP_PD_INVALID_HANDLE;
        testInfo.nrOfPackets = gpBleTest_Config.numberOfTxPackets - testInfo.nrOfPackets;
    }

    GP_LOG_PRINTF("BleTestEnd result %d ",2, testInfo.nrOfPackets);
    return testInfo.nrOfPackets;
}



void gpBleTest_SetContinuousWaveMode( UInt8 newMode )
{
    if( newMode != gpBleTest_Config.cwMode )
    {
        switch( newMode )
        {
            case CW_BLE_MODULATED:
            case CW_BLE_HDRMODULATED:
            case CW_BLE_UNMODULATED:
            {
                gpHal_GoToSleepWhenIdle(false);
                if(gpBleTest_Config.cwMode == CW_OFF)
                {
                    //Previous state was CW_OFF, which means gpHal_GoToSleepWhenIdle(true) was called.
                    gpSched_SetGotoSleepEnable(false);
                }
                // setup to be able to use the direct interface
                gpHal_BleTestDisableBleMgr();
                break;
            }
            case CW_OFF:
            {
                break;
            }
            default:
            {
                 GP_ASSERT_DEV_INT(false);
                 break;
            }
        }

        gpHal_SetContinuousWaveMode(newMode, gpBleTest_Config.blePhysicalChannel, gpBleTest_GetTxPower(),  gpHal_GetBleAntenna());
        GP_LOG_PRINTF("rib channel nr = %d",0,GP_WB_READ_RIB_CHANNEL_NR());
        gpBleTest_Config.cwMode = newMode;
        switch( newMode )
        {
            case CW_OFF:
            {
                gpHal_BleTestReEnableBleMgr();
                // Enabling sleep before setting continuous wave resulted
                // in SPI write failures for k5 due to sleeping
                gpSched_SetGotoSleepEnable(true);
                gpHal_GoToSleepWhenIdle(true);
                break;
            }
            default:
                break;
        }
    }
}

gpHal_ContinuousWaveMode_t gpBleTest_GetContinuousWaveMode(void)
{
    return gpBleTest_Config.cwMode;
}

// deprecated
void gpBleTest_SetModulation(gpBleTest_TxPhy_t modulation)
{
    gpBleTest_SetTxPhy(modulation);
}

void gpBleTest_SetTxPhy(gpBleTest_TxPhy_t txPhy)
{
    GP_ASSERT_DEV_INT(txPhy<gpHal_BleTxPhyInvalid);
    gpBleTest_Config.txPhy = txPhy;
}

gpHal_Result_t gpBleTest_SetRxPhyMask(UInt8 rxPhyMask)
{
    Bool ms_enabled = gpHal_BleGetMultiStandard();
    // check allowed combinations
    Bool phy_2m = BIT_TST( rxPhyMask, gpHal_BleRxPhy2Mb);

    if(ms_enabled && phy_2m)
    {
        return gpHal_ResultInvalidParameter;
    }

    gpBleTest_Config.bleRxPhyMask = rxPhyMask;
    return gpHal_ResultSuccess;
}


/***************************************
 * HAL Callbacks
 **************************************/
void gpBleTest_BleHalDataInd(UInt8 connId, gpPd_Loh_t pdLoh)
{
    if(pdLoh.length == 0)
    {
        // Packets with length 0 should not be processed in the link layer. Possible packets with length zero:
        // 1) Empty PDU's (LLID 1) ==> should normally be dropped in RT subsystem
        // 2) Other LLID values ==> invalid
        GP_LOG_PRINTF("Drop zero length PDU",0);
        gpPd_FreePd(pdLoh.handle);
        return;
    }
#ifdef GP_TEST_NO_RX_INDICATION
    gpPd_FreePd(pdLoh.handle);
#else
    gpTest_cbDataIndication( pdLoh.length, pdLoh.offset, pdLoh.handle );
#endif //GP_TEST_NO_RX_INDICATION

}
