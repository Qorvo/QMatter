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


#include "gpSched.h"
#include "gpHal.h"
#include "gpReset.h"
#include "gpPd.h"
#include "gpBleTest.h"

#include "gpLog.h"
#include "gpBleTestMode.h"
#include "gpBle.h"
#include "gpBleConfig.h"
#include "gpBleDataCommon.h"
#include "gpBle_PhyMask.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLETEST

#define GP_TEST_BLE_NR_OF_TX_PACKETS_DEFAULT    0xFFFF

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
    gpHci_PhyMask_t             bleRxPhyMask;
}gpBleTest_Config_t;

static gpBleTest_Config_t gpBleTest_Config;
/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static gpHal_BleRxPhy_t Test_TxPhyToRxPhy(gpHal_BleTxPhy_t txPhy);
static void gpBleTest_BleHalDataInd(UInt8 connId, gpPd_Loh_t pdLoh);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
gpHal_BleRxPhy_t Test_TxPhyToRxPhy(gpHal_BleTxPhy_t txPhy)
{
    switch (txPhy)
    {
        case gpHal_BleTxPhy1Mb:
            return gpHal_BleRxPhy1Mb;
#ifdef GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        case gpHal_BleTxPhy2Mb:
            return gpHal_BleRxPhy2Mb;
#endif // GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
        default:
            return gpHal_BleRxPhyInvalid;
    }
}


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpBleTest_Init(void)
{
    gpBleTest_Start();
}

void gpBleTest_ResetRequest(void)
{
    gpBle_TestModeReset(false);
}

void gpBleTest_Start(void)
{
    gpHal_EnableDataIndInterrupts(1);
    gpBleTest_Config.blePhysicalChannel = 0xff;
    gpBleTest_Config.txPhy = gpHal_BleTxPhy1Mb;
    gpBleTest_Config.bleRxPhyMask = BleMask_Init(gpHci_Phy_None);
    gpBleTest_Config.testPdHandle = GP_PD_INVALID_HANDLE;
    gpBleTest_Config.numberOfTxPackets = GP_TEST_BLE_NR_OF_TX_PACKETS_DEFAULT;
    gpBleTest_Config.bleRxState = false;
    gpBleTest_Config.accessCode = GPHAL_BLE_DIRECT_TEST_MODE_SYNCWORD;
    gpBleTest_Callbacks.cbDataInd = gpBleTest_BleHalDataInd;
    gpBleTest_Config.cwMode = CW_OFF;
#ifndef GP_COMP_UNIT_TEST
    gpHal_BleRegisterCallbacks(&gpBleTest_Callbacks);
#endif

}

void gpBleTest_Stop(void)
{
    gpBleTest_ResetRequest();
}

void gpBleTest_SetChannel(UInt8 channel)
{
    gpBleTest_Config.blePhysicalChannel = channel;

}

UInt8 gpBleTest_GetChannel(void)
{
    return gpBleTest_Config.blePhysicalChannel;
}

void gpBleTest_SetDeviceAddress(BtDeviceAddress_t* address)
{
    gpHci_CommandParameters_t params;
    gpBle_EventBuffer_t eventBuf;

    MEMCPY(&params.VsdWriteDeviceAddress.address, address, sizeof(BtDeviceAddress_t));
    gpBle_VsdWriteDeviceAddress(&params, &eventBuf);
}

void gpBleTest_GetDeviceAddress(BtDeviceAddress_t* address)
{
    gpHci_CommandParameters_t params;
    gpBle_EventBuffer_t eventBuf;

    (void) gpBle_ReadBdAddr( &params, &eventBuf);
    MEMCPY( address, &eventBuf.payload.commandCompleteParams.returnParams.bdAddress, sizeof(BtDeviceAddress_t));
}

//GP_WB_WRITE_PBM_FORMAT_R_ANTENNA
//GP_WB_WRITE_PBM_BLE_FORMAT_T_ANTSEL_INT
//GP_WB_WRITE_PBM_BLE_FORMAT_T_ANTSEL_EXT
UInt8 gpBleTest_SetTxPower(UInt8 transmitPower)
{
    gpHci_CommandParameters_t params;
    gpBle_EventBuffer_t eventBuf;
    params.VsdSetTransmitPower.transmitPower = transmitPower;
    return gpBle_VsdSetTransmitPower( &params, &eventBuf);
}

Int8 gpBleTest_GetTxPower(void)
{
    Int8 res = gpHal_BleGetTxPower();
    GP_LOG_PRINTF(" TXPOWER %d ",2, res);
    return res;
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

    gpHci_CommandParameters_t hci_params;
    gpBle_EventBuffer_t         eventBuf;

    hci_params.VsdEnhancedReceiverTest.rxchannel = gpBleTest_Config.blePhysicalChannel;
    hci_params.VsdEnhancedReceiverTest.phyMask = gpBleTest_Config.bleRxPhyMask;
    hci_params.VsdEnhancedReceiverTest.modulationIndex = gpHci_ModulationIndex_Standard;
    hci_params.VsdEnhancedReceiverTest.accesscode = gpBleTest_Config.accessCode;
    hci_params.VsdEnhancedReceiverTest.antenna = gpHal_GetBleAntenna();

    gpHci_Result_t hci_result = gpBle_VsdEnhancedReceiverTest(&hci_params, &eventBuf);

    return (hci_result == gpHci_ResultSuccess) ? gpHal_ResultSuccess : gpHal_ResultInvalidParameter;
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
        result = gpBleTest_ReceiverTest();
        GP_ASSERT_DEV_INT(gpHal_ResultSuccess == result);
    }
    else
    {
        gpBleTest_Config.numberOfRxPackets = gpBleTest_TestEnd();
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
    gpHal_Result_t result;
    gpHci_CommandParameters_t   params;
    gpHci_CommandParameters_t   antennaParams;
    gpBle_EventBuffer_t         eventBuf;
    result = (gpBleTest_Config.blePhysicalChannel<= 0x27) ? gpHal_ResultSuccess: gpHal_ResultInvalidParameter;

    if(result == gpHal_ResultSuccess)
    {
        gpHal_AntennaSelection_t antenna = gpHal_GetBleAntenna();
        antennaParams.SetVsdTestParams.value = (UInt8*)&antenna;
        result = gpBle_SetVsdDirectTestModeAntennaHelper(&antennaParams, &eventBuf);
        GP_ASSERT_DEV_INT(gpHal_ResultSuccess == result);

        gpHci_Result_t res;
        params.SetVsdTestParams.value = (UInt8*)&gpBleTest_Config.numberOfTxPackets;
        gpBle_SetVsdDirectTestTxPacketCountHelper(&params, &eventBuf);
        params.LeEnhancedTransmitterTest.phy = gpBleDataCommon_HalPhyWithCodingToHciPhyWithCoding(gpBleTest_Config.txPhy);
        params.LeEnhancedTransmitterTest.txchannel = gpBleTest_Config.blePhysicalChannel;
        params.LeEnhancedTransmitterTest.length = length;
        params.LeEnhancedTransmitterTest.payload=payloadtype;
        res = gpBle_LeEnhancedTransmitterTest(&params, &eventBuf);
        result = (res == gpHci_ResultSuccess) ? gpHal_ResultSuccess : gpHal_ResultInvalidParameter;
    }
    return result;
}


void gpBleTest_AbortTestMode(void)
{
    gpBleTest_TestEnd();
}

UInt16 gpBleTest_TestEnd(void)
{
    gpHci_CommandParameters_t params;
    gpBle_EventBuffer_t eventBuf;

    gpBle_LeTestEnd(&params, &eventBuf);
    GP_LOG_PRINTF(" gpBleTest_TestEnd %d  " ,2, eventBuf.payload.commandCompleteParams.returnParams.testResult );
    return eventBuf.payload.commandCompleteParams.returnParams.testResult;
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
                    hal_SleepSetGotoSleepEnable(false);
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
                hal_SleepSetGotoSleepEnable(true);
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
    gpHal_BleRxPhy_t rxPhy = Test_TxPhyToRxPhy(modulation);
    gpHci_PhyMask_t mask = BleMask_Init(gpBleDataCommon_HalRxPhyToHciPhy(rxPhy));
    GP_LOG_PRINTF("SetModulation mask %u", 0, mask.mask);
    gpHal_Result_t result = gpBleTest_SetRxPhyMask(mask.mask);
    GP_ASSERT_DEV_INT(gpHal_ResultSuccess==result);
}

void gpBleTest_SetTxPhy(gpBleTest_TxPhy_t txPhy)
{
    GP_LOG_PRINTF("gpBleTest_SetTxPhy: %u < %u",0,txPhy,gpHal_BleTxPhyInvalid);
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
        GP_LOG_PRINTF("Warning: ms + 2M is not supported and defaults to 1M",0);
    }

    gpBleTest_Config.bleRxPhyMask.mask = rxPhyMask;
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
