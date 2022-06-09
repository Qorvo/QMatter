/*
 * Copyright (c) 2017-2020, Qorvo Inc
 *
 * radio_qorvo.c
 *   This file contains the implementation of the qorvo radio api for openthread.
 *
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
 *
 */

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP
// #define GP_LOCAL_LOG

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "platform_qorvo.h"
#include <openthread/platform/radio.h>
#include <openthread/platform/diag.h>

#include "global.h"
#include "radio_qorvo.h"
#include "gpMacDispatcher.h"
#include "gpMacCore_defsDefines.h"
#include "gpLog.h"
#include "gpEncryption.h"
#include "gpAssert.h"
#include "gpSched.h"
#include "gpPd.h"

/*****************************************************************************
 *                    Compile Time Verifications
 *****************************************************************************/

#ifndef GP_SCHED_FREE_CPU_TIME
GP_COMPILE_TIME_VERIFY(GP_SCHED_DEFAULT_GOTOSLEEP_THRES>0);
#endif //GP_SCHED_FREE_CPU_TIME

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void gpMacDispatcher_cbDataIndication(const gpMacCore_AddressInfo_t* pSrcAddrInfo, const gpMacCore_AddressInfo_t* pDstAddrInfo, UInt8 dsn,
                                             gpMacCore_Security_t* pSecOptions, gpPd_Loh_t pdLoh, gpMacCore_StackId_t stackId);
#ifdef GP_MACCORE_DIVERSITY_SCAN_ORIGINATOR
static void gpMacDispatcher_cbScanConfirm(gpMacCore_Result_t status, gpMacCore_ScanType_t scanType, UInt32 unscannedChannels,
                                          UInt8 resultListSize, UInt8* pResultList, gpMacCore_StackId_t stackId);
#endif // GP_MACCORE_DIVERSITY_SCAN_ORIGINATOR
static void gpMacDispatcher_cbDataConfirm(gpMacCore_Result_t status, UInt8 msduHandle, gpMacCore_StackId_t stackId);

static void gpMacDispatcher_cbDriverResetIndication(gpMacCore_Result_t status, gpMacCore_StackId_t stackId);

gpMacDispatcher_Callbacks_t mac802154_callbacks = {
    gpMacDispatcher_cbDataIndication,
    gpMacDispatcher_cbDataConfirm,
    NULL, // gpMacDispatcher_cbPollIndication,
    NULL, // gpMacDispatcher_cbPollConfirm,
    NULL, // gpMacDispatcher_cbPurgeConfirm,
    NULL, // gpMacDispatcher_cbBeaconNotifyIndication,
#ifdef GP_MACCORE_DIVERSITY_SCAN_ORIGINATOR
    gpMacDispatcher_cbScanConfirm,
#else
#error "Thread Interfaces MUST perform ED and Active scans"
#endif // GP_MACCORE_DIVERSITY_SCAN_ORIGINATOR
    NULL, // gpMacDispatcher_cbAssocIndication,
    NULL, // gpMacDispatcher_cbAssocConfirm,
    NULL, // gpMacDispatcher_cbOrphanIndication_t
    NULL, // gpMacDispatcher_cbSecurityFailureCommStatusIndication,
    NULL, // gpMacDispatcher_cbAssociateCommStatusIndication_t
    NULL, // gpMacDispatcher_cbOrphanCommStatusIndication_t
    gpMacDispatcher_cbDriverResetIndication,
    NULL, // gpMacDispatcher_cbPollNotify,
    NULL, // gpMacDispatcher_cbSecurityFrameCounterIndication
};

static gpMacCore_StackId_t qorvoGetStackId(void);
static void qorvoSetStackId(gpMacCore_StackId_t stackId);
static bool qorvoValidStackId(gpMacCore_StackId_t stackId);
static otError qorvoToThreadError(gpMacCore_Result_t res);

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static otRadioFrame sReceiveFrame;

static uint8_t sTransmitPsdu[OT_RADIO_FRAME_MAX_SIZE];
static uint8_t sReceivePsdu[OT_RADIO_FRAME_MAX_SIZE];
static otError sTransmitStatus;

#define QVOT_THREAD_1_2_ENABLED 0

// Storage of MAC settings changed by OT
static MACAddress_t qorvoOriginalExtendedMac;
static MACAddress_t qorvoRadioExtendedMac;
static uint16_t qorvoRadioPanId;
static uint16_t qorvoRadioShortAddress;
static bool qorvoRadioRxOnWhenIdle;

static uint8_t qorvoScanResult[1];

#define OPENTHREAD_STRING_IDENTIFIER OTHR
static gpMacCore_StackId_t openThreadStackId = GP_MAC_DISPATCHER_INVALID_STACK_ID;

/*****************************************************************************
 *                    Public Data Definitions
 *****************************************************************************/

// Accessed from radio.c OpenThread glue
otRadioFrame sTransmitFrame;

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

#define QVOT_IS_MAC_2015_FRAME(frameControl)    (gpMacCore_MacVersion2015 == (MACCORE_FRAMECONTROL_FRAMEVERSION_GET((frameControl))))
#define QVOT_IS_ACK_FRAME(frameControl)         (MACCORE_FRAMECONTROL_FRAMETYPE_GET((frameControl)))
#define QVOT_ACKED_WITH_FP(frameControl)        (MACCORE_FRAMECONTROL_FRAMEPENDING_GET((frameControl)))
#define QVOT_SECURITY_ENABLED(frameControl)     (MACCORE_FRAMECONTROL_SECURITY_GET((frameControl)))

gpMacCore_StackId_t qorvoGetStackId(void)
{
    return openThreadStackId;
}

void qorvoSetStackId(gpMacCore_StackId_t stackId)
{
    openThreadStackId = stackId;
}

static bool qorvoValidStackId(gpMacCore_StackId_t stackId)
{
    bool result = true;
    if(stackId != qorvoGetStackId())
    {
        GP_LOG_SYSTEM_PRINTF("stackId %u != qorvoStackId %u", 0, stackId, qorvoGetStackId());
        GP_ASSERT_DEV_INT(0);
        result = false;
    }
    return result;
}

otError qorvoToThreadError(gpMacCore_Result_t res)
{
    switch(res)
    {
        case gpMacCore_ResultSuccess:
        {
            return OT_ERROR_NONE;
            break;
        }
        case gpMacCore_ResultTransactionOverflow:
        {
            return OT_ERROR_BUSY;
            break;
        }
        case gpMacCore_ResultInvalidParameter:
        {
            return OT_ERROR_INVALID_ARGS;
            break;
        }
        case gpMacCore_ResultChannelAccessFailure:
        {
            return OT_ERROR_CHANNEL_ACCESS_FAILURE;
            break;
        }
        case gpMacCore_ResultNoAck:
        {
            return OT_ERROR_NO_ACK;
            break;
        }
        default:
        {
            return OT_ERROR_GENERIC;
            break;
        }
    }
}

/*****************************************************************************
 *                    gpMacDispatcher callbacks
 *****************************************************************************/

void gpMacDispatcher_cbDataIndication(const gpMacCore_AddressInfo_t* pSrcAddrInfo, const gpMacCore_AddressInfo_t* pDstAddrInfo, UInt8 dsn,
                                      gpMacCore_Security_t* pSecOptions, gpPd_Loh_t pdLoh, gpMacCore_StackId_t stackId)
{
    NOT_USED(pSecOptions);
    NOT_USED(pSrcAddrInfo);
    NOT_USED(pDstAddrInfo);
    NOT_USED(dsn);

    if(!qorvoValidStackId(stackId))
    {
        return;
    }

    // Copy packet into OT structure
    gpPd_ReadByteStream(pdLoh.handle, pdLoh.offset, pdLoh.length, &sReceiveFrame.mPsdu[0]);

    sReceiveFrame.mPsdu[pdLoh.length] = 0x00;     // dummy crc byte
    sReceiveFrame.mPsdu[pdLoh.length + 1] = 0x00; // dummy crc byte
    sReceiveFrame.mLength = pdLoh.length + 2;
    sReceiveFrame.mChannel = gpMacDispatcher_GetCurrentChannel(qorvoGetStackId());
    sReceiveFrame.mInfo.mRxInfo.mRssi = gpPd_GetRssi(pdLoh.handle);
    sReceiveFrame.mInfo.mRxInfo.mLqi = gpPd_GetLqi(pdLoh.handle);
    sReceiveFrame.mInfo.mRxInfo.mAckedWithFramePending = true;
    sReceiveFrame.mInfo.mRxInfo.mAckedWithSecEnhAck = false;

    gpPd_FreePd(pdLoh.handle);

    GP_LOG_PRINTF("rx seq = %d", 0, sReceiveFrame.mPsdu[2]);

    {
        cbQorvoRadioReceiveDone(&sReceiveFrame, OT_ERROR_NONE);
    }


}

void gpMacDispatcher_cbDataConfirm(gpMacCore_Result_t status, UInt8 msduHandle, gpMacCore_StackId_t stackId)
{
    if(!qorvoValidStackId(stackId))
    {
        return;
    }

    sTransmitFrame.mChannel = gpPd_GetTxChannel(msduHandle);

    sTransmitStatus = qorvoToThreadError(status);
    {
        bool aFramePending = true;
        uint8_t fp;

        fp = gpPd_GetFramePendingAfterTx(msduHandle);
        /* use the fp value unless it's invalid (0xff), then use 'true' */
        aFramePending = (fp == 0xff) ? true : (fp == 1);

        // gpPd_ReadByteStream(msduHandle, 0, length, sTransmitFrame.mPsdu);

        //GP_LOG_SYSTEM_PRINTF("tx done st:%d pd=%d fp=%d",0, status, msduHandle, aFramePending);
        cbQorvoRadioTransmitDone(&sTransmitFrame, aFramePending, sTransmitStatus);
    }
    gpPd_FreePd(msduHandle);
}

#ifdef GP_MACCORE_DIVERSITY_SCAN_ORIGINATOR
void gpMacDispatcher_cbScanConfirm(gpMacCore_Result_t status, gpMacCore_ScanType_t scanType, UInt32 unscannedChannels,
                                   UInt8 resultListSize, UInt8* pResultList, gpMacCore_StackId_t stackId)
{
    NOT_USED(status);
    NOT_USED(scanType);
    NOT_USED(unscannedChannels);
    NOT_USED(resultListSize);
    NOT_USED(pResultList);

    if(!qorvoValidStackId(stackId))
    {
        return;
    }

    int8_t aEnergyScanMaxRssi = ((UInt8*)pResultList)[0];

    cbQorvoRadioEnergyScanDone(aEnergyScanMaxRssi);
}
#endif // GP_MACCORE_DIVERSITY_SCAN_ORIGINATOR

void gpMacDispatcher_cbDriverResetIndication(gpMacCore_Result_t status, gpMacCore_StackId_t stackId)
{
    NOT_USED(status);

    if(!qorvoValidStackId(stackId))
    {
        return;
    }

    gpMacDispatcher_StringIdentifier_t openThreadStringId = {{XSTRINGIFY(OPENTHREAD_STRING_IDENTIFIER)}};

    //register NWK layer to MAC layer
    qorvoSetStackId(gpMacDispatcher_RegisterNetworkStack(&openThreadStringId));
    gpMacDispatcher_RegisterCallbacks(qorvoGetStackId(), &mac802154_callbacks);

    qorvoRadioReset();
    gpMacDispatcher_SetRxOnWhenIdle(qorvoRadioRxOnWhenIdle, qorvoGetStackId());
    gpMacDispatcher_SetExtendedAddress(&qorvoRadioExtendedMac, qorvoGetStackId());
    gpMacDispatcher_SetShortAddress(qorvoRadioShortAddress, qorvoGetStackId());
    gpMacDispatcher_SetPanId(qorvoRadioPanId, qorvoGetStackId());
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void qorvoRadioReset(void)
{
    gpMacDispatcher_Reset(true, qorvoGetStackId());

    GP_LOG_PRINTF("otst=%d bl=%x", 0, qorvoGetStackId(), 0xFF);
    gpMacDispatcher_SetStackInRawMode(1, qorvoGetStackId());
    gpMacDispatcher_EnableEnhancedAck(QVOT_THREAD_1_2_ENABLED, qorvoGetStackId());

    gpMacDispatcher_SetTransactionPersistenceTime(0, qorvoGetStackId());
    gpMacDispatcher_SetMacVersion(gpMacCore_MacVersion2006, qorvoGetStackId());
}

void qorvoRadioInit(void)
{
    gpMacDispatcher_StringIdentifier_t openThreadStringId = {{XSTRINGIFY(OPENTHREAD_STRING_IDENTIFIER)}};

    sTransmitFrame.mLength = 0;
    sTransmitFrame.mPsdu = sTransmitPsdu;
    sReceiveFrame.mLength = 0;
    sReceiveFrame.mPsdu = sReceivePsdu;

    //register NWK layer to MAC layer
    qorvoSetStackId(gpMacDispatcher_RegisterNetworkStack(&openThreadStringId));
    gpMacDispatcher_RegisterCallbacks(qorvoGetStackId(), &mac802154_callbacks);

    qorvoRadioReset();

    // Save initial IEEE MAC address
    gpMacDispatcher_GetExtendedAddress(&qorvoOriginalExtendedMac, qorvoGetStackId());
    MEMCPY(&qorvoRadioExtendedMac, &qorvoOriginalExtendedMac, sizeof(MACAddress_t));
    qorvoRadioShortAddress = 0XFFFE;
    qorvoRadioPanId = 0xFFFE;

    // Set sleep behavior
    gpSched_SetGotoSleepEnable(true);

}

otError qorvoRadioTransmit(otRadioFrame* aFrame)
{
    UInt8 offset = 0;
    gpPd_Loh_t pdLoh;
    gpMacCore_Security_t secOptions;
    gpMacCore_MultiChannelOptions_t multiChannelOptions;
    gpMacCore_AddressMode_t srcAddrMode = 0x02;
    gpMacCore_AddressInfo_t dstAddrInfo;
    UInt8 txOptions = 0;

    pdLoh.handle = gpPd_GetPd();
    if(pdLoh.handle == GP_PD_INVALID_HANDLE)
    {
        GP_LOG_SYSTEM_PRINTF("no more pd handles:%x !", 0, pdLoh.handle);
        return OT_ERROR_NO_BUFS;
    }

    pdLoh.length = aFrame->mLength - offset;
    pdLoh.length -= 2; // drop the 2 crc bytes

    pdLoh.offset = 0;
    gpPd_WriteByteStream(pdLoh.handle, pdLoh.offset, pdLoh.length, aFrame->mPsdu + offset);

    //multiChannelOptions.channel[0] = local->phy->current_channel;
    if (aFrame->mChannel < 11 || aFrame->mChannel > 26)
    {
        GP_ASSERT_DEV_INT(aFrame->mChannel >= 11);
        GP_ASSERT_DEV_INT(aFrame->mChannel <= 26);
        return OT_ERROR_INVALID_ARGS;
    }
    multiChannelOptions.channel[0] = aFrame->mChannel;
    multiChannelOptions.channel[1] = GP_MACCORE_INVALID_CHANNEL;
    multiChannelOptions.channel[2] = GP_MACCORE_INVALID_CHANNEL;

    MEMSET(&secOptions, 0, sizeof(gpMacCore_Security_t));

    // The Security bit in the frameControl bit is set, but OpenThread has not yet handled the security
    if(QVOT_SECURITY_ENABLED(aFrame->mPsdu[0]) && !aFrame->mInfo.mTxInfo.mIsSecurityProcessed)
    {
        secOptions.securityLevel = gpEncryption_SecLevelENC_MIC32;
    }
    else
    {
        // No absolute need to set it, the memset took care or this, but it's clearer
        secOptions.securityLevel = gpEncryption_SecLevelNothing;
    }
    dstAddrInfo.addressMode = 0x03;
    txOptions = GP_MACCORE_TX_OPT_RAW;

    UInt8 retries = min(aFrame->mInfo.mTxInfo.mMaxFrameRetries, 7);
    gpMacDispatcher_SetNumberOfRetries(retries, qorvoGetStackId());
    gpMacDispatcher_SetMaxCsmaBackoffs(aFrame->mInfo.mTxInfo.mMaxCsmaBackoffs, qorvoGetStackId());
    gpMacDispatcher_DataRequest(srcAddrMode, &dstAddrInfo, txOptions, &secOptions, multiChannelOptions, pdLoh, qorvoGetStackId());

    return OT_ERROR_NONE;
}

void qorvoRadioProcess(void)
{
}

void qorvoRadioGetIeeeEui64(uint8_t* aIeeeEui64)
{
    for(UInt8 i = 0; i < OT_EXT_ADDRESS_SIZE; i++)
    {
        ((UInt8*)aIeeeEui64)[i] = ((UInt8*)(&qorvoOriginalExtendedMac))[7 - i];
    }
    GP_LOG_PRINTF("otPlatRadioGetIeeeEui64 = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 0,
                  aIeeeEui64[0], aIeeeEui64[1], aIeeeEui64[2], aIeeeEui64[3],
                  aIeeeEui64[4], aIeeeEui64[5], aIeeeEui64[6], aIeeeEui64[7]);
}

void qorvoRadioSetCurrentChannel(uint8_t channel)
{
    gpMacDispatcher_SetCurrentChannel(channel, qorvoGetStackId());
}

void qorvoRadioSetRxOnWhenIdle(bool rxOnWhenIdle)
{
    qorvoRadioRxOnWhenIdle = rxOnWhenIdle;
    gpMacDispatcher_SetRxOnWhenIdle(rxOnWhenIdle, qorvoGetStackId());
}

void qorvoRadioSetPanId(uint16_t panid)
{
    GP_LOG_PRINTF("otPlatRadioSetPanId = %02x", 0, panid);
    qorvoRadioPanId = panid;
    gpMacDispatcher_SetPanId(panid, qorvoGetStackId());
}

void qorvoRadioSetExtendedAddress(const uint8_t* address)
{
    MEMCPY(&qorvoRadioExtendedMac, address, sizeof(MACAddress_t));
    GP_LOG_PRINTF("otPlatRadioSetExtendedAddress = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 0,
                  address[0], address[1], address[2], address[3],
                  address[4], address[5], address[6], address[7]);
    gpMacDispatcher_SetExtendedAddress(&qorvoRadioExtendedMac, qorvoGetStackId());
}

void qorvoRadioSetShortAddress(uint16_t address)
{
    GP_LOG_PRINTF("otPlatRadioSetShortAddress = %02x", 0, address);
    qorvoRadioShortAddress = address;
    gpMacDispatcher_SetShortAddress(address, qorvoGetStackId());
}

void qorvoRadioEnableSrcMatch(bool aEnable)
{
    gpMacCore_Result_t res;

    if(aEnable)
    {
        res = gpMacDispatcher_SetDataPendingMode(gpMacCore_DataPendingModeMac802154, qorvoGetStackId());
        GP_ASSERT_DEV_INT(res == gpMacCore_ResultSuccess);
    }
    else
    {
        res = gpMacDispatcher_SetDataPendingMode(gpMacCore_DataPendingModeForNonNeighbourDevices, qorvoGetStackId());
        GP_ASSERT_DEV_INT(res == gpMacCore_ResultSuccess);
    }
}

otError qorvoRadioAddSrcMatchShortEntry(const uint16_t aShortAddress, uint16_t panid)
{
    gpMacCore_Result_t res;
    gpMacCore_AddressInfo_t addrInfo;

    addrInfo.addressMode = gpMacCore_AddressModeShortAddress;
    addrInfo.panId = panid;
    addrInfo.address.Short = aShortAddress;

    res = gpMacDispatcher_DataPending_QueueAdd(&addrInfo, qorvoGetStackId());
    GP_LOG_PRINTF("[Q-OT] Add SrcMatchEntry (res: %u): panid: %04x, addr: %04x", 0,
                  res, panid, aShortAddress);
    return qorvoToThreadError(res);
}

otError qorvoRadioAddSrcMatchExtEntry(const uint8_t* aExtAddress, uint16_t panid)
{
    gpMacCore_Result_t res;
    gpMacCore_AddressInfo_t addrInfo;

    addrInfo.addressMode = gpMacCore_AddressModeExtendedAddress;
    addrInfo.panId = panid;
    MEMCPY(&addrInfo.address.Extended, aExtAddress, sizeof(MACAddress_t));

    res = gpMacDispatcher_DataPending_QueueAdd(&addrInfo, qorvoGetStackId());
    GP_LOG_PRINTF("[Q-OT] Add SrcMatchEntry (res: %u): panid: %04x, addr: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 0,
                  res, panid,
                  aExtAddress[0], aExtAddress[1], aExtAddress[2], aExtAddress[3],
                  aExtAddress[4], aExtAddress[5], aExtAddress[6], aExtAddress[7]);
    return qorvoToThreadError(res);
}

otError qorvoRadioClearSrcMatchShortEntry(const uint16_t aShortAddress, uint16_t panid)
{
    gpMacCore_Result_t res;
    gpMacCore_AddressInfo_t addrInfo;

    addrInfo.addressMode = gpMacCore_AddressModeShortAddress;
    addrInfo.panId = panid;
    addrInfo.address.Short = aShortAddress;

    res = gpMacDispatcher_DataPending_QueueRemove(&addrInfo, qorvoGetStackId());
    GP_LOG_PRINTF("[Q-OT] Del SrcMatchEntry (res: %u): panid: %04x, addr: %04x", 0,
                  res, panid, aShortAddress);
    return qorvoToThreadError(res);
}

otError qorvoRadioClearSrcMatchExtEntry(const uint8_t* aExtAddress, uint16_t panid)
{
    gpMacCore_Result_t res;
    gpMacCore_AddressInfo_t addrInfo;

    addrInfo.addressMode = gpMacCore_AddressModeExtendedAddress;
    addrInfo.panId = panid;
    MEMCPY(&addrInfo.address.Extended, aExtAddress, sizeof(MACAddress_t));

    res = gpMacDispatcher_DataPending_QueueRemove(&addrInfo, qorvoGetStackId());
    GP_LOG_PRINTF("[Q-OT] Del SrcMatchEntry (res: %u): panid: %04x, addr: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 0,
                  res, panid,
                  aExtAddress[0], aExtAddress[1], aExtAddress[2], aExtAddress[3],
                  aExtAddress[4], aExtAddress[5], aExtAddress[6], aExtAddress[7]);
    return qorvoToThreadError(res);
}

void qorvoRadioClearSrcMatchEntries(void)
{
    /* clear both short and extended addresses here */
    GP_LOG_PRINTF("[Q-OT] Clear all SrcMatchEntries", 0);
    gpMacDispatcher_DataPending_QueueClear(qorvoGetStackId());
}

otError qorvoRadioGetTransmitPower(int8_t* aPower)
{
    UInt8 channel = gpMacDispatcher_GetCurrentChannel(qorvoGetStackId());
    if(channel == GP_MACCORE_INVALID_CHANNEL)
    {
        return OT_ERROR_INVALID_STATE;
    }
    *aPower = gpMacDispatcher_GetTransmitPower(qorvoGetStackId());
    return OT_ERROR_NONE;
}

otError qorvoRadioSetTransmitPower(int8_t aPower)
{
    UInt8 channel = gpMacDispatcher_GetCurrentChannel(qorvoGetStackId());
    if(channel == GP_MACCORE_INVALID_CHANNEL)
    {
        return OT_ERROR_INVALID_STATE;
    }
    gpMacDispatcher_SetTransmitPower(aPower, qorvoGetStackId());
    return OT_ERROR_NONE;
}

otError qorvoRadioEnergyScan(uint8_t aScanChannel, uint16_t aScanDuration)
{
    UInt32 scanChannels = 1 << aScanChannel;
    UInt8 scanDuration = 0;
    UInt32 durationInUs = (((UInt32)GP_MACCORE_BASE_SUPERFRAME_DURATION * ((UInt32)(((UInt16)1) << scanDuration) + 1)) * GP_MACCORE_SYMBOL_DURATION);
    UInt8 resultListSize = 1;
    UInt8* pResultList = qorvoScanResult;

    GP_ASSERT_DEV_INT(scanDuration < 16);

    while((durationInUs < ((UInt32)aScanDuration * 1000)) && (scanDuration < 15))
    {
        scanDuration++;
        durationInUs = (((UInt32)GP_MACCORE_BASE_SUPERFRAME_DURATION * ((UInt32)(((UInt16)1) << scanDuration) + 1)) * GP_MACCORE_SYMBOL_DURATION);
    }

    gpMacDispatcher_ScanRequest(gpMacCore_ScanTypeED, scanChannels, scanDuration, resultListSize, pResultList, qorvoGetStackId());

    return OT_ERROR_NONE;
}

bool qorvoRadioGetPromiscuous(void)
{
    return gpMacDispatcher_GetPromiscuousMode(qorvoGetStackId());
}

void qorvoRadioSetPromiscuous(bool aEnable)
{
    gpMacDispatcher_SetPromiscuousMode(aEnable, qorvoGetStackId());
}



otError qorvoRadioEnableCsl(uint32_t       aCslPeriod,
                            uint16_t       aShortAddr,
                            const uint8_t *aExtAddr)
{
    NOT_USED(aCslPeriod);
    NOT_USED(aShortAddr);
    NOT_USED(aExtAddr);
    return OT_ERROR_NOT_IMPLEMENTED;
}

void qorvoRadioUpdateCslSampleTime(uint32_t aCslSampleTime)
{
    // Not implemented
    NOT_USED(aCslSampleTime);
}

otError qorvoRadioConfigureEnhAckProbing(otLinkMetrics  aLinkMetrics,
                                         uint16_t       aShortAddress,
                                         const uint8_t *aExtAddress)
{
    NOT_USED(aLinkMetrics);
    NOT_USED(aShortAddress);
    NOT_USED(aExtAddress);
    return OT_ERROR_NOT_IMPLEMENTED;
}
