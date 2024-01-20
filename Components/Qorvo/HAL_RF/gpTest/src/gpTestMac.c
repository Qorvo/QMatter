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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpBsp.h"
#include "hal.h"

#include "gpSched.h"
#include "gpHal.h"
#include "gpReset.h"
#include "gpRandom.h"
#include "gpPd.h"
#include "gpHal_MAC_Ext.h"
#include "gpStat.h"
#include "gpRxArbiter.h"

#ifdef GP_DIVERSITY_LOG
#include "gpLog.h"
#endif /* GP_DIVERSITY_LOG */
#include "gpTestMac.h"
/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_TEST

#define TEST_RX_ARBITER_STACK_ID            0

#define VALID_SLEEP_MODE                    true
#define INVALID_SLEEP_MODE                  false
/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#define DEFAULT_DATA_LENGTH 15
#define MAX_DATA_LENGTH 128

static UInt8 DataExpectedRxPacket[DEFAULT_DATA_LENGTH] FLASH_PROGMEM = {0x1,0x00,0xAA,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C}; //{0x01, 0x88, 0xAA, 0x5E, 0xC0, 0xDF, 0xEB, 0x5E, 0xC0, 0xDE, 0xEB, 0x0, 0x0, 0x0, 0x0};          // {0x1, 0x98, 0xAA, 0xFE, 0xCA, 0xEF, 0xBE, 0xFE, 0xCA, 0xEE, 0xBE, 0x0, 0x0};
static UInt8 MacRxExpectedPacket_dataLength = sizeof(DataExpectedRxPacket);


static const UInt8 ROM DataToSendDefault[DEFAULT_DATA_LENGTH] FLASH_PROGMEM = {0x1,0x00,0xAA,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C};

//We can also use poolmem if needed
UInt8 gpTest_DataToTransmit[MAX_DATA_LENGTH];

gpTest_Statistics_t gpTest_Statistics;

UInt8   gpTest_CurrentTxChannel;
extern gpTest_ContinuousWaveMode_t gpTest_ContinuousWaveMode;
gpHal_CollisionAvoidanceMode_t gpTest_CollisionAvoidanceModeToUse = gpHal_CollisionAvoidanceModeNoCCA; // CSMA-mode used in TX

// variables used for multiple commands (TX and ED)
UInt16  gpTest_RepeatCounter;
UInt16  gpTest_Interval;

// TX related variables
UInt8   gpTest_SendOptions;
UInt8   gpTest_LengthDataToSend;
UInt16  gpTest_TxPacketsOK;
UInt16  gpTest_TxPacketsError;
UInt16  gpTest_IncrementingCounter;
UInt16  gpTest_PacketsInTheAir;
// handles used for packet Tx
gpPd_Loh_t gpTest_pdLoh_alternate = {0x0, 0x0, GP_PD_INVALID_HANDLE};
gpPd_Loh_t gpTest_pdLoh = {0x0, 0x0, GP_PD_INVALID_HANDLE};
gpPad_Handle_t gpTest_PadHandle;
// EDScan param
UInt16  gpTest_ChannelMask;
UInt32  gpTest_duration_us;
#ifdef GP_TEST_RANDOM_WINDOW_US
UInt16 gpTest_PseudoRandomCounter;
#endif

static Bool gpTest_EDActive;

static gpHal_MacScenario_t gpTest_macScenario = gpHal_MacDefault;

static UInt32 MacSetRxResponsePacket_delayUs;
static UInt8 MacSetRxResponsePacket_dataLength = 0;
static UInt8 MacSetRxResponsePacket_data[GP_TEST_MAX_LENGTH_PACKET];
static UInt8 MacSetRxResponsePacket_txOptions;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void    Test_CallEDRequest(void);
static void    Test_SendData(void);
static void    Test_BuildRandomPacket(UInt8 length);
static void    Test_BuildRandomPacketPd(UInt8 length, gpPd_Loh_t pdLoh);
static void    Test_BuildIncrementingPacket(void);

static void gpTestHal_cbMacEDConfirm_dummy(UInt16 channelMask, UInt8 *proto_energy_level_list);
void gpTestHal_cbMacDataConfirm(UInt8 status, gpPd_Loh_t pdLoh, UInt8 lastChannelUsed);
void gpTestHal_cbMacDataIndication(gpPd_Loh_t pdLoh, gpHal_RxInfo_t *rxInfo);
void gpTestHal_cbMacEDConfirm(UInt16 channel, UInt8 *proto_energy_level);


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

static void gpTest_Common_MacTxPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions, gpPd_Loh_t *pdHandle);
static void gpTest_TxResponse(void);

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
void gpTest_MacRegisterCallbacks(void)
{
    gpHal_RegisterDataConfirmCallback(gpTestHal_cbMacDataConfirm);
    gpHal_RegisterDataIndicationCallback(gpTestHal_cbMacDataIndication);
    gpHal_RegisterEDConfirmCallback(gpTestHal_cbMacEDConfirm);
}
void gpTest_MacRegisterSnifferCallback(Bool flag)
{
    NOT_USED(flag);
}

void gpTest_MacUnregisterCallbacks(void)
{
     gpHal_RegisterDataConfirmCallback(NULL);
    gpHal_RegisterDataIndicationCallback(NULL);
    gpHal_RegisterEDConfirmCallback(NULL);
}
void gpTest_MacInit(void)
{
    gpPad_Attributes_t padInitAttributes;
    gpTest_EDActive = false;
    gpTest_CurrentTxChannel = 17;
    MEMCPY_P(gpTest_DataToTransmit, DataToSendDefault, DEFAULT_DATA_LENGTH);
    gpTest_LengthDataToSend = DEFAULT_DATA_LENGTH;
    gpTest_ResetStatistics();
    gpTest_RepeatCounter = 0;
    gpTest_Interval = 0;
    gpTest_SendOptions = 0;
    gpTest_TxPacketsOK = 0;
    gpTest_TxPacketsError = 0;
#ifdef GP_TEST_RANDOM_WINDOW_US
    gpTest_PseudoRandomCounter = 0;
#endif
    gpTest_ContinuousWaveMode = CW_OFF;
    gpTest_IncrementingCounter = 0;
    gpHal_RegisterEDConfirmCallback(gpTestHal_cbMacEDConfirm);
        padInitAttributes.channels[0] = gpTest_CurrentTxChannel;
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
        padInitAttributes.retransmitOnCcaFail = false;
        padInitAttributes.retransmitRandomBackoff = false;
        padInitAttributes.minBERetransmit   = 0;
        padInitAttributes.maxBERetransmit   = 5;
     if (gpPad_CheckPadValid(gpTest_PadHandle) != gpPad_ResultValidHandle)
    {
        gpTest_PadHandle = gpPad_GetPad(&padInitAttributes);
    }
    else
    {
  //      GP_LOG_SYSTEM_PRINTF("REINIT",0);
        gpPad_SetAttributes(gpTest_PadHandle, &padInitAttributes); //Re-init old handle
    }
    GP_ASSERT_SYSTEM(gpPad_CheckPadValid(gpTest_PadHandle) == gpPad_ResultValidHandle);
//    GP_LOG_SYSTEM_PRINTF("Test pdHandle %x",0,(UInt16) gpTest_PadHandle);
}

gpTest_TxPower_t gpTest_MacGetLastUsedTxPower(void)
{
    return gpHal_GetLastUsedTxPower();
}
void gpTest_MacSetAntennaDiversity(Bool OnOff)
{
    if(OnOff == true)
    {
        gpTest_SetAntenna(gpHal_AntennaSelection_Auto);
    }
    else
    {
        gpTest_SetAntenna(gpHal_AntennaSelection_Ant0);
    }
}
Bool gpTest_MacGetAntennaDiversity(void)
{
    return gpHal_GetRxAntennaDiversity();
}
void gpTest_MacStart(void)
{
    gpTest_MacInit();
    gpTest_MacRegisterCallbacks();
    gpRxArbiter_ResetStack(TEST_RX_ARBITER_STACK_ID);
    (void)gpRxArbiter_SetStackPriority(0,TEST_RX_ARBITER_STACK_ID);

    //Get Pd and Pad
    if(gpPd_CheckPdValid(gpTest_pdLoh.handle) != gpPd_ResultValidHandle)
    {
        gpTest_pdLoh.handle = gpPd_GetPd();
    }
    GP_ASSERT_SYSTEM(gpPd_CheckPdValid(gpTest_pdLoh.handle) == gpPd_ResultValidHandle);

    if(gpPd_CheckPdValid(gpTest_pdLoh_alternate.handle) != gpPd_ResultValidHandle)
    {
        gpTest_pdLoh_alternate.handle = gpPd_GetPd();
    }
    GP_ASSERT_SYSTEM(gpPd_CheckPdValid(gpTest_pdLoh_alternate.handle) == gpPd_ResultValidHandle);

    gpTest_CurrentTxChannel = 20;

    {
        gpPad_Attributes_t padInitAttributes;

        padInitAttributes.channels[0] = gpTest_CurrentTxChannel;
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
        //K7 only support energy, k5 can also do modulated
        padInitAttributes.cca = gpHal_CCAModeEnergy;
        padInitAttributes.retransmitOnCcaFail = false;
        padInitAttributes.retransmitRandomBackoff = false;
        padInitAttributes.minBERetransmit   = 0;
        padInitAttributes.maxBERetransmit   = 5;

        if (gpPad_CheckPadValid(gpTest_PadHandle) != gpPad_ResultValidHandle)
        {
            gpTest_PadHandle = gpPad_GetPad(&padInitAttributes);
        }
        else
        {
            gpPad_SetAttributes(gpTest_PadHandle, &padInitAttributes); //Re-init old handle
        }
        GP_ASSERT_SYSTEM(gpPad_CheckPadValid(gpTest_PadHandle) == gpPad_ResultValidHandle);
    }

    gpRxArbiter_SetStackChannel(gpTest_CurrentTxChannel , TEST_RX_ARBITER_STACK_ID);

    gpTest_MacSetMaxBE(5);
    gpTest_MacSetMinBE(3);
    gpTest_MacSetMaxCSMABackoffs(4);
    gpTest_MacSetNumberOfRetries(3);
    gpTest_SetCollisionAvoidanceModeToUse(gpHal_CollisionAvoidanceModeNoCCA);
    gpTest_SetPacketInPacketMode(1);
    gpTest_SetRxState(false);
    gpTest_SetPromiscuousMode(true);
    gpTest_SetShortAddress(0xFFFF, gpHal_SourceIdentifier_0);
    gpTest_SetPanId(0xCAFE, gpHal_SourceIdentifier_0);

    // Enable interrupts
    gpHal_EnableInterrupts(true);
    gpHal_EnablePrimitiveCallbackInterrupt(true);

    gpTest_MacSetTxPower(gpPad_GetTxPower(gpTest_PadHandle));
}

void gpTest_MacStop(void)
{

    gpSched_UnscheduleEvent(Test_SendData);

    //Register gpHal callbacks to our own test functions
    gpHal_RegisterDataConfirmCallback(NULL);
    gpHal_RegisterDataIndicationCallback(NULL);
    gpHal_RegisterEDConfirmCallback((gpTest_EDActive) ? gpTestHal_cbMacEDConfirm_dummy : NULL);

    gpRxArbiter_ResetStack(TEST_RX_ARBITER_STACK_ID);

    //Get Pd and Pad
    if(gpPd_CheckPdValid(gpTest_pdLoh.handle) == gpPd_ResultValidHandle)
    {
        gpPd_FreePd(gpTest_pdLoh.handle);
    }

    if(gpPd_CheckPdValid(gpTest_pdLoh_alternate.handle) == gpPd_ResultValidHandle)
    {
        gpPd_FreePd(gpTest_pdLoh_alternate.handle);
    }
    if (gpPad_CheckPadValid(gpTest_PadHandle) == gpPad_ResultValidHandle)
    {
        // init hardware (needs to be done before freeing pad)
        gpTest_SetAntennaDiversity(true);
        gpTest_SetAntenna(gpHal_AntennaSelection_Auto);

        gpPad_FreePad(gpTest_PadHandle);
    }

    gpTest_SetPromiscuousMode(false);

}

void gpTest_MacSetMaxBE(UInt8 maxBE)
{
    gpPad_SetTxMaxBE(gpTest_PadHandle, maxBE);
}

void gpTest_MacSetMinBE(UInt8 minBE)
{
    gpPad_SetTxMinBE(gpTest_PadHandle, minBE);
}

void gpTest_MacSetMaxCSMABackoffs(UInt8 maxBackoffs)
{
    gpPad_SetTxMaxCsmaBackoffs(gpTest_PadHandle,maxBackoffs);
}

void gpTest_MacSetNumberOfRetries(UInt8 retries)
{
    gpPad_SetTxMaxFrameRetries(gpTest_PadHandle,retries);
}

UInt8 gpTest_MacGetNumberOfRetries(void)
{
    return gpPad_GetTxMaxFrameRetries(gpTest_PadHandle);
}

void gpTest_MacSetRxState(Bool flag )
{
    gpRxArbiter_RadioState_t currentState = gpRxArbiter_GetCurrentRxOnState();
    if((currentState>0) != flag) gpRxArbiter_SetStackRxOn(flag, TEST_RX_ARBITER_STACK_ID);
}

Bool gpTest_MacGetRxState(void)
{
    gpRxArbiter_RadioState_t currentState = gpRxArbiter_GetCurrentRxOnState();
    return (currentState>0);
}

gpTest_Result_t gpTest_MacSetChannel(UInt8 channel)
{
    gpRxArbiter_Result_t res;
    res = gpRxArbiter_SetStackChannel(channel , TEST_RX_ARBITER_STACK_ID);
    // only store channel if it is successfully set
    if (res == gpRxArbiter_ResultSuccess)
    {
        gpTest_CurrentTxChannel = channel;
    }
    return (res==gpRxArbiter_ResultSuccess)?gpHal_ResultSuccess:gpHal_ResultInvalidParameter;
}

UInt8 gpTest_MacGetChannel(void)
{
    return gpTest_CurrentTxChannel;
}

//gpHal_AntennaSelection_t gpTest_GetRxAntenna(void) - mapped directly

void gpTest_MacSetTxPower(Int8 transmitPower)
{
    UIntLoop i;
    Int8 pDefaultTxPowers[16];


    for (i = 0; i < 16; i++)
    {
        pDefaultTxPowers[i] = transmitPower;
    }

    //For ACK Tx - power that will be used by gphal by default + when specying 'Default' enum
    gpHal_SetDefaultTransmitPowers(pDefaultTxPowers);
    //For Tx + Power value fetched from this var for CW
    gpPad_SetTxPower(gpTest_PadHandle, transmitPower);
}

Int8 gpTest_MacGetTxPower(void)
{
   return gpPad_GetTxPower(gpTest_PadHandle);
}

UInt8 gpTest_MacGetMaxBE(void)
{
    return gpPad_GetTxMaxBE(gpTest_PadHandle);
}

UInt8 gpTest_MacGetMinBE(void)
{
    return gpPad_GetTxMinBE(gpTest_PadHandle);
}

UInt8 gpTest_MacGetMaxCSMABackoffs(void)
{
    return gpPad_GetTxMaxCsmaBackoffs(gpTest_PadHandle);
}

void gpTest_MacSetContinuousWaveMode(gpTest_ContinuousWaveMode_t newMode)
{
    if(newMode != gpTest_ContinuousWaveMode)
    {

        switch(newMode)
        {
            case CW_MODULATED:
            case CW_UNMODULATED:
            {
                if(gpTest_ContinuousWaveMode == CW_OFF)
                {
                    //Previous state was CW_OFF, which means gpHal_GoToSleepWhenIdle(true) was called.
                    gpHal_GoToSleepWhenIdle(false);
                    hal_SleepSetGotoSleepEnable(false);
                }
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
        gpHal_SetContinuousWaveMode(newMode, gpTest_CurrentTxChannel, gpPad_GetTxPower(gpTest_PadHandle), gpPad_GetTxAntenna(gpTest_PadHandle));
        gpTest_ContinuousWaveMode = newMode;
        switch(newMode)
        {
            case CW_OFF:
            {
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

gpTest_ContinuousWaveMode_t gpTest_MacGetContinuousWaveMode(void)
{
    return gpTest_ContinuousWaveMode;
}

Bool gpTest_MacGetContTx(void)
{
    if(gpTest_RepeatCounter == 0xffff)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void gpTest_Common_MacTxPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions, gpPd_Loh_t *gpTest_pdLoh)
{
    Bool randomData = txOptions & GP_TEST_TXOPTIONS_RANDOMDATA;
    Bool useIncrementingCounter = txOptions & GP_TEST_TXOPTIONS_INCREMENTINGCTR;
    GP_ASSERT_SYSTEM(gpTest_pdLoh);
    gpTest_SendOptions = txOptions; // save it so that the reader knows how to compare
    if(gpTest_ContinuousWaveMode != CW_OFF)
    {
        gpTest_cbDataConfirm(gpHal_ResultBusy, 0, 0);
        return;
    }

    if(numberOfPackets == 0)
    {
        // we want to stop continuous tx, just set the counter to 0 and be done with it
        gpTest_RepeatCounter = 0;
        if (!useIncrementingCounter)
        {
            return;
        }
    }

    //Reset Tx counters
    gpTest_TxPacketsOK = 0;
    gpTest_TxPacketsError = 0;

    if((dataLength <= GP_TEST_MAX_LENGTH_PACKET) && gpPd_ResultValidHandle == gpPd_CheckPdValid(gpTest_pdLoh->handle)) //Maybe not yet initialized
    {
        // always save the length, is also used for random length packets
        gpTest_pdLoh->length = gpTest_LengthDataToSend;
        gpTest_pdLoh->offset = 0;
        gpTest_RepeatCounter = numberOfPackets;
        gpTest_Interval = intervalInMs;
        //gpTest_ApplyIncrementingCounter = useIncrementingCounter;

        // Need some code here for the incrementing counter data
        if(randomData == false)
        {
            if (!useIncrementingCounter)       // legacy code
            {
                //If pData == NULL, we will use the default frame, already copied in the buffer in the Init
                if(pData != NULL)
                {
                    gpTest_LengthDataToSend = dataLength;
                    gpTest_pdLoh->length = gpTest_LengthDataToSend;
                    MEMCPY(gpTest_DataToTransmit, pData, gpTest_LengthDataToSend);
                }
                gpPd_WriteByteStream(gpTest_pdLoh->handle,gpTest_pdLoh->offset,gpTest_pdLoh->length,gpTest_DataToTransmit);
            }
            else    // introduced for PER test in LST: some data with an incrementing counter added (in the last 2 bytes)
            {
                // Retrieve the initial value of the incrementing counter; the number of packets is in gpTest_RepeatCounter already
                if (dataLength <= MAX_DATA_LENGTH)
                {
                    gpTest_LengthDataToSend = dataLength;
                    gpTest_pdLoh->length = gpTest_LengthDataToSend;
                    MEMCPY(gpTest_DataToTransmit, pData, gpTest_LengthDataToSend);
                }
                MEMCPY(&gpTest_IncrementingCounter/*ui16*/, (UInt8 *)(&gpTest_DataToTransmit[gpTest_LengthDataToSend-sizeof(UInt16)]), sizeof(UInt16));
                //not done for gpTest_RepeatCounter neither HOST_TO_LITTLE_UINT16_CONVERSION(gpTest_IncrementingCounter);   // = HOST_TO_LITTLE_UINT16_CONVERSION(ui16);
                //gpPd_WriteByteStream(gpTest_pdLoh.handle,gpTest_pdLoh.offset,gpTest_pdLoh.length,gpTest_DataToTransmit);
            }
        }

        //Start transmission
        Test_SendData();
    }
    else
    {
        gpTest_cbDataConfirm(gpHal_ResultInvalidParameter, 0, 0);
    }
}

void gpTest_MacTxPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions)
{
    gpTest_macScenario = gpHal_MacDefault;
    gpTest_Common_MacTxPacket(numberOfPackets, intervalInMs, dataLength, pData, txOptions, &gpTest_pdLoh);
}

void gpTest_MacTxCorruptedPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions)
{
    Bool bForceCorrectPacket = txOptions & GP_TEST_TXOPTIONS_AUTOMATICALLY_CORRECT_PACKET;
    txOptions &= ~GP_TEST_TXOPTIONS_AUTOMATICALLY_CORRECT_PACKET;
    gpTest_macScenario = gpHal_MacManualCrc_NoRetries; //manual CRC expected but don't add it toe the pData so it results in corrupt packet
    gpTest_Common_MacTxPacket(1, intervalInMs, dataLength, pData, txOptions, &gpTest_pdLoh);
    if (bForceCorrectPacket)
    {
        gpTest_macScenario = gpHal_MacDefault; //manual CRC expected but don't add it toe the pData so it results in corrupt packet
        gpTest_Common_MacTxPacket(1, 100, dataLength, pData, txOptions, &gpTest_pdLoh_alternate);        // gpTest_pdLoh still in use, use alternate
    }
}

void gpTest_MacTxPollPacket(UInt16 numberOfPackets, UInt16 intervalInMs, UInt8 dataLength, UInt8* pData, UInt8 txOptions)
{
    gpTest_macScenario = gpHal_MacPollReq;
    gpTest_Common_MacTxPacket(numberOfPackets, intervalInMs, dataLength, pData, txOptions, &gpTest_pdLoh);
}

void gpTest_MacSetRxResponsePacket(UInt32 delayUs, UInt8 dataLength, UInt8* pData, UInt8 txOptions)
{
    MacSetRxResponsePacket_delayUs = delayUs;
    MacSetRxResponsePacket_dataLength = dataLength < sizeof(MacSetRxResponsePacket_data) ? dataLength : sizeof(MacSetRxResponsePacket_data);
    MEMCPY(MacSetRxResponsePacket_data, pData, MacSetRxResponsePacket_dataLength);
    MacSetRxResponsePacket_txOptions = txOptions;
}

void gpTest_MacEDScan(UInt16 numberOfScans, UInt16 intervalInMs, UInt16 channelMask, UInt32 duration_us)
{
    if(gpTest_ContinuousWaveMode != CW_OFF)
    {
        gpTest_cbEDConfirm(gpHal_ResultBusy, (UInt16)0x0000, NULL, true);
        return;
    }

    gpTest_RepeatCounter = numberOfScans;
    gpTest_Interval = intervalInMs;
    gpTest_ChannelMask = channelMask;
    gpTest_duration_us = duration_us;

    Test_CallEDRequest();
}
void gpTest_MacSetCollisionAvoidanceModeToUse(gpTest_CollisionAvoidanceMode_t newMode)
{
    gpTest_CollisionAvoidanceModeToUse = newMode;
}


gpTest_CollisionAvoidanceMode_t gpTest_MacGetCollisionAvoidanceModeInUse(void)
{
    return gpTest_CollisionAvoidanceModeToUse;
}

UInt8 gpTest_MacGetAverageLQI(void)
{
    if(gpTest_Statistics.ReceivedPacketCounterD != 0)
    {
        return((UInt8) (gpTest_Statistics.CumulativeLQI/gpTest_Statistics.ReceivedPacketCounterD));
    }
    else
    {
        return 0;
    }
}


Int8 gpTest_MacGetAverageRSSI(void)
{
    if(gpTest_Statistics.ReceivedPacketCounterD != 0)
    {
        return((Int8) (gpTest_Statistics.CumulativeRssi/gpTest_Statistics.ReceivedPacketCounterD));
    }
    else
    {
        return 0;
    }
}


/********************************************************************************************
*            HAL (gpHal callback functions)                                            *
*********************************************************************************************/

void gpTestHal_cbMacDataConfirm(UInt8 status, gpPd_Loh_t pdLoh, UInt8 lastChannelUsed)
{
    GP_STAT_SAMPLE_TIME(); //gpHal_cbDataConfirm return

    if(status == gpHal_ResultSuccess)
    {
        gpTest_Statistics.PacketsSentOK++;
        gpTest_TxPacketsOK++;
    }
    else
    {
        gpTest_Statistics.PacketsSentError++;
        gpTest_TxPacketsError++;
    }

    if(gpTest_RepeatCounter != 0xffff && gpTest_RepeatCounter != 0)
    {
        gpTest_RepeatCounter--;
    }

    //Duplicated code from send test because here it is using the same pd that comes from the callback instead of using the gpTest_pdLoh
    if(gpTest_Interval == GP_TEST_INTERVAL_NO_DELAY)
    {
        gpTest_PacketsInTheAir--;

        if(gpTest_RepeatCounter != 0)
        {
            gpHal_Result_t gpTestHalResult;
            gpHal_DataReqOptions_t  dataReqOptions;
            dataReqOptions.macScenario = gpTest_macScenario;
            dataReqOptions.srcId       = TEST_RX_ARBITER_STACK_ID;

            // data to be sent has been set using function gpTest_SetTxData()
            if(gpTest_SendOptions & GP_TEST_TXOPTIONS_RANDOMDATA)
            {
                Test_BuildRandomPacketPd(gpTest_LengthDataToSend, pdLoh);
            }
            else if (gpTest_SendOptions & GP_TEST_TXOPTIONS_INCREMENTINGCTR)
            {
                MEMCPY((UInt8 *)(&gpTest_DataToTransmit[gpTest_LengthDataToSend-sizeof(UInt16)]), (UInt8 *)&gpTest_IncrementingCounter, sizeof(UInt16));
                Test_BuildIncrementingPacket();
                gpTest_IncrementingCounter++;  //don't care overflow
            }

            gpTestHalResult = gpHal_DataRequest(&dataReqOptions, gpTest_PadHandle, pdLoh);

            if(gpTestHalResult == gpHal_ResultSuccess)
            {
                gpTest_PacketsInTheAir++;
            }
            else
            {
                gpPd_FreePd(pdLoh.handle);
            }
        }
        else
        {
            gpPd_FreePd(pdLoh.handle);
        }

        if (gpTest_PacketsInTheAir == 0)
        {
            gpTest_cbDataConfirm(status, gpTest_TxPacketsOK, gpTest_TxPacketsError);
            //Restore initial buffer
            MEMCPY_P(gpTest_DataToTransmit, DataToSendDefault, DEFAULT_DATA_LENGTH);
            gpTest_LengthDataToSend = DEFAULT_DATA_LENGTH;
        }
        return;
    }

    if(gpTest_RepeatCounter)
    {
        // start another transmit
#ifndef GP_TEST_RANDOM_WINDOW_US
        gpSched_ScheduleEvent(MS_TO_US(gpTest_Interval), Test_SendData);
#else
        gpSched_ScheduleEvent(MS_TO_US(gpTest_Interval) + gpTest_PseudoRandomCounter, Test_SendData);
        gpTest_PseudoRandomCounter = (gpTest_PseudoRandomCounter + (GP_TEST_RANDOM_WINDOW_US/2 - 1)) % GP_TEST_RANDOM_WINDOW_US;
#endif
    }
    else
    {
        gpTest_cbDataConfirm(status, gpTest_TxPacketsOK, gpTest_TxPacketsError);
        //Restore default
        MEMCPY_P(gpTest_DataToTransmit, DataToSendDefault, DEFAULT_DATA_LENGTH);
        gpTest_LengthDataToSend = DEFAULT_DATA_LENGTH;
    }
}

void gpTestHal_cbMacDataIndication(gpPd_Loh_t pdLoh, gpHal_RxInfo_t *rxInfo)
{
    UInt8 psdu[GP_TEST_MAX_LENGTH_PACKET];
    UInt8 length;
    NOT_USED(rxInfo);

    GP_STAT_SAMPLE_TIME();

    length = ((pdLoh.length <= GP_TEST_MAX_LENGTH_PACKET) ? pdLoh.length : GP_TEST_MAX_LENGTH_PACKET); //Clip length
    gpPd_ReadByteStream(pdLoh.handle,pdLoh.offset,length,psdu);


    // always maintain all packets counters, regardless of setting
    gpTest_Statistics.ReceivedPacketCounterC++;
    if(pdLoh.length == MacRxExpectedPacket_dataLength)
    {
        gpTest_Statistics.ReceivedPacketCounterL++;

        if (gpTest_SendOptions & GP_TEST_TXOPTIONS_INCREMENTINGCTR)
        {
            MEMCPY((UInt8 *)(&gpTest_DataToTransmit[gpTest_LengthDataToSend-sizeof(UInt16)]), (UInt8 *)&gpTest_IncrementingCounter, sizeof(UInt16));
            gpTest_IncrementingCounter++;
        }
        { //always
            if(!MEMCMP((void*)DataExpectedRxPacket, (void*)psdu, pdLoh.length))
            {
                UInt8 Lqi;
                Int8 Rssi;

                gpTest_Statistics.ReceivedPacketCounterD++;

                Lqi = gpPd_GetLqi(pdLoh.handle);
                Rssi = gpPd_GetRssi(pdLoh.handle);

                gpTest_Statistics.CumulativeLQI += Lqi;
                gpTest_Statistics.CumulativeRssi += Rssi;
            }
            else if (gpTest_SendOptions & GP_TEST_TXOPTIONS_INCREMENTINGCTR)
            { // MISMATCH -- possibly lost some frames - try to resync the expected counter with the received data
                if(!MEMCMP((void*)gpTest_DataToTransmit, (void*)psdu, pdLoh.length-sizeof(UInt16)))  // constant part is still OK?
                {
                    // hedg debug
                    UInt16 origCounter;
                    MEMCPY((UInt8 *)&origCounter, (UInt8 *)&(gpTest_DataToTransmit[pdLoh.length-sizeof(origCounter)]), sizeof(origCounter));
                    MEMCPY((UInt8 *)&gpTest_IncrementingCounter, (UInt8 *)(&psdu[gpTest_LengthDataToSend-sizeof(UInt16)]),  sizeof(UInt16));
                    gpTest_IncrementingCounter++;   // next packet the counter will be incremented again
                }
            }
        }
    }

#ifdef GP_TEST_NO_RX_INDICATION
    gpPd_FreePd(pdLoh.handle);
#else
    gpTest_cbDataIndication(pdLoh.length, pdLoh.offset, pdLoh.handle);
#endif //GP_TEST_NO_RX_INDICATION

    if (0 < MacSetRxResponsePacket_dataLength)
    {
        gpSched_ScheduleEvent(MacSetRxResponsePacket_delayUs, gpTest_TxResponse);
    }
}

void gpTestMac_SetExpectedRx(UInt8 dataLength, UInt8* pData)
{

    if (dataLength > sizeof(DataExpectedRxPacket))
    {
        return;
    }
    MacRxExpectedPacket_dataLength = dataLength;
    MEMCPY(DataExpectedRxPacket, pData, MacRxExpectedPacket_dataLength);
}


void gpTest_TxResponse(void)
{
    GP_ASSERT_DEV_INT (0 < MacSetRxResponsePacket_dataLength);

    gpTest_Common_MacTxPacket(1, 0, MacSetRxResponsePacket_dataLength, MacSetRxResponsePacket_data, MacSetRxResponsePacket_txOptions, &gpTest_pdLoh);
}

static void gpTestHal_cbMacEDConfirm_dummy(UInt16 channelMask, UInt8 *proto_energy_level_list)
{
    NOT_USED(channelMask);
    NOT_USED(proto_energy_level_list);
    gpTest_EDActive = false;
}

void gpTestHal_cbMacEDConfirm(UInt16 channelMask, UInt8 *proto_energy_level_list)
{
    UInt8 idx;
    UInt8 pData[16];

    for (idx=0; idx<16; ++idx)
    {
        pData[idx] = gpHal_CalculateED(proto_energy_level_list[idx]);
    }

    if (gpTest_RepeatCounter > 0)
    {
        gpTest_RepeatCounter--;
        if(gpTest_RepeatCounter)
        {
            gpSched_ScheduleEvent(MS_TO_US(gpTest_Interval), Test_CallEDRequest);
        }

        gpTest_cbEDConfirm(gpHal_ResultSuccess, channelMask, pData, (gpTest_RepeatCounter == 0));
        if (gpTest_RepeatCounter == 0) {
             gpTest_EDActive = false;
        }
    }
    else
    {
#ifdef GP_DIVERSITY_LOG
        GP_LOG_SYSTEM_PRINTF("!!! Attempt to call confirm more than requested times!", 0);
#endif //GP_DIVERSITY_LOG
    }
}

void Test_SendData(void)
{
    gpHal_Result_t gpTestHalResult = gpHal_ResultSuccess;
    gpHal_DataReqOptions_t  dataReqOptions;
    UInt8 channels[3] = {gpTest_CurrentTxChannel, GP_HAL_MULTICHANNEL_INVALID_CHANNEL, GP_HAL_MULTICHANNEL_INVALID_CHANNEL};

    dataReqOptions.macScenario = gpTest_macScenario;
    dataReqOptions.srcId       = TEST_RX_ARBITER_STACK_ID;
#ifdef GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION
    dataReqOptions.rawEncryptionEnable = false;
    dataReqOptions.rawKeepFrameCounter = false;
#endif //def GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION
    gpPad_SetTxCsmaMode(gpTest_PadHandle, gpTest_CollisionAvoidanceModeToUse);
    gpPad_SetTxChannels(gpTest_PadHandle, channels);

    // data to be sent has been set using function gpTest_SetTxData()
    if(gpTest_SendOptions & GP_TEST_TXOPTIONS_RANDOMDATA)
    {
        Test_BuildRandomPacket(gpTest_LengthDataToSend);
    }
    else if (gpTest_SendOptions & GP_TEST_TXOPTIONS_INCREMENTINGCTR)
    {
        MEMCPY((UInt8 *)(&gpTest_DataToTransmit[gpTest_LengthDataToSend-sizeof(UInt16)]), (UInt8 *)&gpTest_IncrementingCounter, sizeof(UInt16));
        Test_BuildIncrementingPacket();
        gpTest_IncrementingCounter++;  //don't care overflow
    }

    GP_STAT_SAMPLE_TIME(); //gpHal_DataRequest start

    if(gpTest_Interval != GP_TEST_INTERVAL_NO_DELAY)
    {
        gpTestHalResult = gpHal_DataRequest(&dataReqOptions, gpTest_PadHandle, gpTest_pdLoh);
    }
    else
    {
        if(gpTest_RepeatCounter < 4)
        {
            gpTest_cbDataConfirm(gpHal_ResultInvalidParameter, 0, 0);
            return;
        }

        gpTest_RepeatCounter = gpTest_RepeatCounter - 3;
    }

    gpTest_PacketsInTheAir = 0;
    if (gpTest_TxPacketsOK == 0 && gpTest_TxPacketsError == 0)
    {
        if(gpTest_Interval == GP_TEST_INTERVAL_NO_DELAY)
        {
            UInt8 i;

            for(i = 0; i < 4; i++)
            {
                gpPd_Loh_t newPdLoh;
                newPdLoh.handle = gpPd_GetPd();
                if (gpPd_CheckPdValid(newPdLoh.handle) == gpPd_ResultValidHandle)
                {
                    newPdLoh.length = gpTest_pdLoh.length;
                    newPdLoh.offset = gpTest_pdLoh.offset;
                    if( gpTest_SendOptions & GP_TEST_TXOPTIONS_RANDOMDATA)
                    {
                        Test_BuildRandomPacketPd(gpTest_LengthDataToSend, newPdLoh);
                    }
                    else
                    {
                        gpPd_WriteByteStream(newPdLoh.handle,newPdLoh.offset,newPdLoh.length,gpTest_DataToTransmit);
                    }
                    gpTestHalResult = gpHal_DataRequest(&dataReqOptions, gpTest_PadHandle, newPdLoh);

                    if(gpTestHalResult == gpHal_ResultSuccess)
                    {
                        gpTest_PacketsInTheAir++;
                    }
                }
            }
        }
    }
    if((gpTestHalResult != gpHal_ResultSuccess) && (gpTest_PacketsInTheAir == 0))
    {
        gpTest_cbDataConfirm(gpTestHalResult, gpTest_Statistics.PacketsSentOK, gpTest_Statistics.PacketsSentError);
    }
    gpTest_macScenario = gpHal_MacDefault;

}

void Test_CallEDRequest(void)
{
    gpTest_EDActive = true;
    gpHal_EDRequest(gpTest_duration_us, gpTest_ChannelMask);
}


void Test_BuildRandomPacket(UInt8 length)
{
    UInt8 i;

    // make sure header is there..
    gpPd_WriteByte(gpTest_pdLoh.handle,0,0x01);
    gpPd_WriteByte(gpTest_pdLoh.handle,1,0x00);

    if(length < 3)
    {
        gpTest_LengthDataToSend = 0;
        while(gpTest_LengthDataToSend<3 || gpTest_LengthDataToSend>=125)
        {
            gpRandom_GetNewSequence(1, &gpTest_LengthDataToSend);
        }
    }
    else
    {
    gpTest_LengthDataToSend = length;
    }

    for(i=2; i<gpTest_LengthDataToSend; i++)
    {
         UInt8 randomByte;

         gpRandom_GetNewSequence(1, &randomByte);

        gpPd_WriteByte(gpTest_pdLoh.handle,i,randomByte);
    }
}

void Test_BuildRandomPacketPd(UInt8 length, gpPd_Loh_t pdLoh)
{
    UInt8 randomByte;
    UInt8 i;

    // make sure header is there..
    gpPd_WriteByte(pdLoh.handle,0,0x01);
    gpPd_WriteByte(pdLoh.handle,1,0x00);

    for( i=2; i<gpTest_LengthDataToSend; i++ )
    {

#ifdef HAL_DIVERSITY_RANDOM
        HAL_GET_RANDOM_BYTES(sizeof(randomByte), (void*)&randomByte);
#else
        gpRandom_GetNewSequence(1, &randomByte);
#endif

        gpPd_WriteByte(pdLoh.handle,i,randomByte);
    }
}

void Test_BuildIncrementingPacket()
{
    UInt8 i;

    for(i=0; i<gpTest_LengthDataToSend; i++)
    {
        gpPd_WriteByte(gpTest_pdLoh.handle,i,gpTest_DataToTransmit[i]);
    }
}

void gpTest_MacGetSettings(gpTest_Settings_t * Settings)
{
    if(Settings)
    {
        Settings->AntennaMode = gpTest_MacGetAntennaDiversity();
        Settings->SelectedAntenna = gpPad_GetTxAntenna(gpTest_PadHandle);
        Settings->SelectedChannel = gpTest_MacGetChannel();
        Settings->ContinuousWave = gpTest_MacGetContinuousWaveMode();
        Settings->MaxBE = gpTest_MacGetMaxBE();
        Settings->MinBE = gpTest_MacGetMinBE();
        Settings->MaxCSMABackoffs = gpTest_MacGetMaxCSMABackoffs();
        Settings->MaxRetries = gpTest_MacGetNumberOfRetries();
        Settings->CSMAMode = gpTest_MacGetCollisionAvoidanceModeInUse();
        Settings->PacketInPacket = gpTest_GetPacketInPacketMode();
        Settings->RxOnWhenIdle = gpTest_MacGetRxState();
        Settings->PowerSetting = gpTest_MacGetTxPower();
        Settings->PromiscuousMode = gpTest_MacGetPromiscuousMode();
        Settings->PanID = gpTest_MacGetPanId(gpHal_SourceIdentifier_0);
        Settings->ShortAddress = gpTest_MacGetShortAddress(gpHal_SourceIdentifier_0);
    }
}


void gpTest_MacGetStatistics(gpTest_Statistics_t * Statistics)
{
    MEMCPY(Statistics, &gpTest_Statistics, sizeof(gpTest_Statistics_t));
}


void gpTest_MacResetStatistics(void)
{
    MEMSET(&gpTest_Statistics, 0, sizeof(gpTest_Statistics_t));
}

gpTest_Result_t gpTest_MacSetChannelForStack(UInt8 stackId, UInt8 channel)
{
    /* for stackId 0 you are supposed to use the function gpTest_SetChannel*/
    GP_ASSERT_SYSTEM((stackId==1) || (stackId==2));

    if(channel == GP_HAL_MULTICHANNEL_INVALID_CHANNEL)
    {
        gpHal_SetRxOnWhenIdle(stackId, false, channel);
    }
    else
    {
        gpHal_SetRxOnWhenIdle(stackId, true, channel);
    }
    return ((stackId==1) || (stackId==2))?gpHal_ResultSuccess:gpHal_ResultInvalidParameter;
}

void gpTest_MacSetTxAntenna(gpTest_AntennaSelection_t antenna)
{
#ifdef GP_HAL_DIVERSITY_SINGLE_ANTENNA
    antenna = GP_HAL_DIVERSITY_SINGLE_ANTENNA;
#endif
    if(antenna == gpHal_AntennaSelection_Ant0 || antenna == gpHal_AntennaSelection_Ant1)
    {
        gpPad_SetTxAntenna(gpTest_PadHandle, antenna);
    }
}

gpTest_AntennaSelection_t gpTest_MacGetTxAntenna()
{
    return gpPad_GetTxAntenna(gpTest_PadHandle);
}

Bool gpTest_MacGetPromiscuousMode(void)
{
    return gpHal_GetPromiscuousMode();
}
UInt16 gpTest_MacGetPanId(gpHal_SourceIdentifier_t srcId)
{
    return gpHal_GetPanId(srcId);
}
UInt16 gpTest_MacGetShortAddress(gpHal_SourceIdentifier_t srcId)
{
    return gpHal_GetShortAddress(srcId);
}

void gpTest_MacSetRetransmitOnCcaFail(Bool enable)
{
    gpPad_SetRetransmitOnCcaFail(gpTest_PadHandle, enable);
}
Bool gpTest_MacGetRetransmitOnCcaFail(void)
{
    return gpPad_GetRetransmitOnCcaFail(gpTest_PadHandle);
}

void gpTest_MacSetRetransmitRandomBackoff(Bool enable)
{
    gpPad_SetRetransmitRandomBackoff(gpTest_PadHandle, enable);
}
Bool gpTest_MacGetRetransmitRandomBackoff(void)
{
    return gpPad_GetRetransmitRandomBackoff(gpTest_PadHandle);
}

void gpTest_MacSetMinBeRetransmit(UInt8 minBERetransmit)
{
    gpPad_SetMinBeRetransmit(gpTest_PadHandle, minBERetransmit);
}
UInt8 gpTest_MacGetMinBeRetransmit(void)
{
    return gpPad_GetMinBeRetransmit(gpTest_PadHandle);
}
void gpTest_MacSetMaxBeRetransmit(UInt8 maxBERetransmit)
{
    gpPad_SetMaxBeRetransmit(gpTest_PadHandle, maxBERetransmit);
}
UInt8 gpTest_MacGetMaxBeRetransmit(void)
{
    return gpPad_GetMaxBeRetransmit(gpTest_PadHandle);
}
