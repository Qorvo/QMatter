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
 * $Header: //depot/release/Embedded/Applications/P959_OpenThread/v1.1.23.1/comps/qvOT/src/radio_qorvo.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
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

#include "radio_qorvo.h"
#include "gpMacDispatcher.h"
#include "gpLog.h"
#include "gpEncryption.h"
#include "gpAssert.h"
#include "gpSched.h"
#include "gpPd.h"

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void gpMacDispatcher_cbDataIndication(const gpMacCore_AddressInfo_t* pSrcAddrInfo, const gpMacCore_AddressInfo_t* pDstAddrInfo, UInt8 dsn,
                                             gpMacCore_Security_t* pSecOptions, gpPd_Loh_t pdLoh, gpMacCore_StackId_t stackId);
static void gpMacDispatcher_cbBeaconNotifyIndication(UInt8 bsn, gpMacCore_PanDescriptor_t* pPanDescriptor, UInt8 beaconPayloadLength,
                                                     UInt8* pBeaconPayload, gpMacCore_StackId_t stackId);
static void gpMacDispatcher_cbPollNotify(gpMacCore_AddressInfo_t* coordAddrInfo, gpPd_TimeStamp_t txTime, gpPd_Handle_t pdHandle, Bool fromNeighbour, gpMacCore_StackId_t stackId);

static void gpMacDispatcher_cbSecurityFailureCommStatusIndication(gpMacCore_AddressInfo_t* pSrcAddrInfo, gpMacCore_AddressInfo_t* pDstAddrInfo, gpMacCore_Result_t status, gpPd_TimeStamp_t txTime, gpMacCore_StackId_t stackId);

static void gpMacDispatcher_cbSecurityFrameCounterIndication(UInt32 frameCounter, gpMacCore_StackId_t stackId);

static void gpMacDispatcher_cbPurgeConfirm(gpMacCore_Result_t status, gpPd_Handle_t pdHandle, gpMacCore_StackId_t stackId);

static void gpMacDispatcher_cbScanConfirm(gpMacCore_Result_t status, gpMacCore_ScanType_t scanType, UInt32 unscannedChannels,
                                          UInt8 resultListSize, UInt8* pResultList, gpMacCore_StackId_t stackId);
static void gpMacDispatcher_cbDataConfirm(gpMacCore_Result_t status, UInt8 msduHandle, gpMacCore_StackId_t stackId);

static void gpMacDispatcher_cbPollConfirm(gpMacDispatcher_Result_t status, gpMacCore_AddressInfo_t* pAddrInfo, gpPd_TimeStamp_t txTime, gpMacCore_StackId_t stackId);

static void gpMacDispatcher_cbDriverResetIndication(gpMacCore_Result_t status, gpMacCore_StackId_t stackId);

gpMacDispatcher_Callbacks_t mac802154_callbacks = {
    gpMacDispatcher_cbDataIndication,
    gpMacDispatcher_cbDataConfirm,
#ifdef GP_MACCORE_DIVERSITY_POLL_RECIPIENT
    NULL, //gpMacDispatcher_cbPollIndication,
#else
    NULL,
#endif // GP_MACCORE_DIVERSITY_POLL_RECIPIENT
#ifdef GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
    gpMacDispatcher_cbPollConfirm,
#else
    NULL,
#endif // GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
    gpMacDispatcher_cbPurgeConfirm,
    gpMacDispatcher_cbBeaconNotifyIndication,
    gpMacDispatcher_cbScanConfirm,
    NULL, // gpMacDispatcher_cbAssocIndication,
    NULL, // gpMacDispatcher_cbAssocConfirm,
    NULL, // gpMacDispatcher_cbOrphanIndication_t
    gpMacDispatcher_cbSecurityFailureCommStatusIndication,
    NULL, // gpMacDispatcher_cbAssociateCommStatusIndication_t
    NULL, // gpMacDispatcher_cbOrphanCommStatusIndication_t
    gpMacDispatcher_cbDriverResetIndication,
    gpMacDispatcher_cbPollNotify,
    gpMacDispatcher_cbSecurityFrameCounterIndication
};

static gpMacCore_StackId_t qorvoGetStackId(void);
static otError qorvoToThreadError(gpMacCore_Result_t res);

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

enum {
    IEEE802154_MIN_LENGTH = 5,
    IEEE802154_MAX_LENGTH = 127,
    IEEE802154_ACK_LENGTH = 5,
    IEEE802154_FRAME_TYPE_MASK = 0x7,
    IEEE802154_FRAME_TYPE_ACK = 0x2,
    IEEE802154_FRAME_PENDING = 1 << 4,
    IEEE802154_ACK_REQUEST = 1 << 5,
    IEEE802154_DSN_OFFSET = 2,
};

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static otRadioFrame sReceiveFrame;

static uint8_t sTransmitPsdu[IEEE802154_MAX_LENGTH];
static uint8_t sReceivePsdu[IEEE802154_MAX_LENGTH];

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

gpMacCore_StackId_t qorvoGetStackId(void)
{
    return openThreadStackId;
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

    // Copy packet into OT structure
    gpPd_ReadByteStream(pdLoh.handle, pdLoh.offset, pdLoh.length, &sReceiveFrame.mPsdu[0]);
    sReceiveFrame.mPsdu[pdLoh.length] = 0x00;     // dummy crc byte
    sReceiveFrame.mPsdu[pdLoh.length + 1] = 0x00; // dummy crc byte
    sReceiveFrame.mLength = pdLoh.length + 2;
    sReceiveFrame.mInfo.mRxInfo.mRssi = gpPd_GetRssi(pdLoh.handle);
    sReceiveFrame.mInfo.mRxInfo.mLqi = gpPd_GetLqi(pdLoh.handle);
    sReceiveFrame.mInfo.mRxInfo.mAckedWithFramePending = true;
    sReceiveFrame.mChannel = gpMacDispatcher_GetCurrentChannel(qorvoGetStackId());

    gpPd_FreePd(pdLoh.handle);

    GP_LOG_PRINTF("rx seq = %d", 0, sReceiveFrame.mPsdu[2]);
    //GP_LOG_SYSTEM_PRINTF("offset = %d length = %d channel=%d",0, pdLoh.offset, pdLoh.length, sReceiveFrame.mChannel);
    //gpLog_PrintBuffer(pdLoh.length + offset, sReceiveFrame.mPsdu);

    cbQorvoRadioReceiveDone(&sReceiveFrame, OT_ERROR_NONE);
}

void gpMacDispatcher_cbDataConfirm(gpMacCore_Result_t status, UInt8 msduHandle, gpMacCore_StackId_t stackId)
{
    bool aFramePending = true;
    uint8_t fp;

    fp = gpPd_GetFramePendingAfterTx(msduHandle);
    /* use the fp value unless it's invalid (oxff), then use 'true' */
    aFramePending = (fp == 0xff) ? true : (fp == 1);

    //GP_LOG_SYSTEM_PRINTF("tx done st:%d pd=%d fp=%d",0, status, msduHandle, aFramePending);
    cbQorvoRadioTransmitDone(&sTransmitFrame, aFramePending, qorvoToThreadError(status));
    gpPd_FreePd(msduHandle);
}

void gpMacDispatcher_cbBeaconNotifyIndication(UInt8 bsn, gpMacCore_PanDescriptor_t* pPanDescriptor, UInt8 beaconPayloadLength,
                                              UInt8* pBeaconPayload, gpMacCore_StackId_t stackId)
{
    NOT_USED(bsn);
    NOT_USED(pPanDescriptor);
    NOT_USED(beaconPayloadLength);
    NOT_USED(pBeaconPayload);
}

void gpMacDispatcher_cbPollNotify(gpMacCore_AddressInfo_t* coordAddrInfo, gpPd_TimeStamp_t txTime, gpPd_Handle_t pdHandle, Bool fromNeighbour, gpMacCore_StackId_t stackId)
{
    NOT_USED(coordAddrInfo);
    NOT_USED(txTime);
    NOT_USED(pdHandle);
    NOT_USED(fromNeighbour);
}

void gpMacDispatcher_cbScanConfirm(gpMacCore_Result_t status, gpMacCore_ScanType_t scanType, UInt32 unscannedChannels,
                                   UInt8 resultListSize, UInt8* pResultList, gpMacCore_StackId_t stackId)
{
    NOT_USED(status);
    NOT_USED(scanType);
    NOT_USED(unscannedChannels);
    NOT_USED(resultListSize);
    NOT_USED(pResultList);

    int8_t aEnergyScanMaxRssi = ((UInt8*)pResultList)[0];

    cbQorvoRadioEnergyScanDone(aEnergyScanMaxRssi);
}

void gpMacDispatcher_cbSecurityFailureCommStatusIndication(gpMacCore_AddressInfo_t* pSrcAddrInfo, gpMacCore_AddressInfo_t* pDstAddrInfo,
                                                           gpMacCore_Result_t status, gpPd_TimeStamp_t txTime, gpMacCore_StackId_t stackId)
{
    NOT_USED(pSrcAddrInfo);
    NOT_USED(pDstAddrInfo);
    NOT_USED(status);
    NOT_USED(txTime);
}

void gpMacDispatcher_cbSecurityFrameCounterIndication(UInt32 frameCounter, gpMacCore_StackId_t stackId)
{
    NOT_USED(frameCounter);
}

void gpMacDispatcher_cbPurgeConfirm(gpMacCore_Result_t status, gpPd_Handle_t pdHandle, gpMacCore_StackId_t stackId)
{
    NOT_USED(status);
    NOT_USED(pdHandle);
}

void gpMacDispatcher_cbPollConfirm(gpMacDispatcher_Result_t status, gpMacCore_AddressInfo_t* pAddrInfo, gpPd_TimeStamp_t txTime, gpMacCore_StackId_t stackId)
{
    NOT_USED(status);
    NOT_USED(pAddrInfo);
    NOT_USED(txTime);
}

void gpMacDispatcher_cbDriverResetIndication(gpMacCore_Result_t status, gpMacCore_StackId_t stackId)
{
    gpMacDispatcher_StringIdentifier_t openThreadStringId = {{XSTRINGIFY(OPENTHREAD_STRING_IDENTIFIER)}};

    //register NWK layer to MAC layer
    openThreadStackId = gpMacDispatcher_RegisterNetworkStack(&openThreadStringId);
    gpMacDispatcher_RegisterCallbacks(openThreadStackId, &mac802154_callbacks);

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
    gpMacDispatcher_Reset(true, openThreadStackId);
    GP_LOG_PRINTF("otst=%d bl=%x", 0, openThreadStackId, 0xFF);
    gpMacDispatcher_SetBeaconPayloadLength(0xFF, openThreadStackId);

    gpMacDispatcher_SetTransactionPersistenceTime(0, openThreadStackId);
    gpMacDispatcher_SetMacVersion(gpMacCore_MacVersion2006, openThreadStackId);
}

void qorvoRadioInit(void)
{
    gpMacDispatcher_StringIdentifier_t openThreadStringId = {{XSTRINGIFY(OPENTHREAD_STRING_IDENTIFIER)}};

    sTransmitFrame.mLength = 0;
    sTransmitFrame.mPsdu = sTransmitPsdu;
    sReceiveFrame.mLength = 0;
    sReceiveFrame.mPsdu = sReceivePsdu;

    //register NWK layer to MAC layer
    openThreadStackId = gpMacDispatcher_RegisterNetworkStack(&openThreadStringId);
    gpMacDispatcher_RegisterCallbacks(openThreadStackId, &mac802154_callbacks);

    qorvoRadioReset();

    // Save initial IEEE MAC address
    gpMacDispatcher_GetExtendedAddress(&qorvoOriginalExtendedMac, qorvoGetStackId());
    MEMCPY(&qorvoRadioExtendedMac, &qorvoOriginalExtendedMac, sizeof(MACAddress_t));
    qorvoRadioShortAddress = 0XFFFE;
    qorvoRadioPanId = 0xFFFE;

    // Set sleep behavior
#ifdef GP_SCHED_FREE_CPU_TIME
    /* linux / rpi case */
    gpSched_SetGotoSleepEnable(false);
#else
    /* embedded case */
    gpSched_SetGotoSleepEnable(true);
#endif //GP_SCHED_FREE_CPU_TIME

}

otError qorvoRadioTransmit(otRadioFrame* aPacket)
{
    otError error = OT_ERROR_INVALID_STATE;

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
    GP_LOG_PRINTF("Tx on channel %d seq = %d", 0, aPacket->mChannel, aPacket->mPsdu[2]);
    //GP_LOG_SYSTEM_PRINTF("tx mac len:%d pd=%d",0, aPacket->mLength, pdLoh.handle);
    //gpLog_PrintBuffer(aPacket->mLength, aPacket->mPsdu);

    pdLoh.length = aPacket->mLength - offset;
    pdLoh.length -= 2; // drop the 2 crc bytes

    pdLoh.offset = 0;
    gpPd_WriteByteStream(pdLoh.handle, pdLoh.offset, pdLoh.length, aPacket->mPsdu + offset);

    //multiChannelOptions.channel[0] = local->phy->current_channel;
    GP_ASSERT_DEV_INT(aPacket->mChannel >= 11);
    GP_ASSERT_DEV_INT(aPacket->mChannel <= 26);
    multiChannelOptions.channel[0] = aPacket->mChannel;
    multiChannelOptions.channel[1] = GP_MACCORE_INVALID_CHANNEL;
    multiChannelOptions.channel[2] = GP_MACCORE_INVALID_CHANNEL;

    memset(&secOptions, 0, sizeof(gpMacCore_Security_t));
    secOptions.securityLevel = gpEncryption_SecLevelNothing;
    dstAddrInfo.addressMode = 0x03;
    txOptions = GP_MACCORE_TX_OPT_RAW;

    gpMacDispatcher_DataRequest(srcAddrMode, &dstAddrInfo, txOptions, &secOptions, multiChannelOptions, pdLoh, qorvoGetStackId());

    error = OT_ERROR_NONE;

    return error;
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
    MACAddress_t extendedMac;

    for(UInt8 i = 0; i < OT_EXT_ADDRESS_SIZE; i++)
    {
        ((UInt8*)&extendedMac)[i] = address[i];
    }
    MEMCPY(&qorvoRadioExtendedMac, address, sizeof(MACAddress_t));
    GP_LOG_PRINTF("otPlatRadioSetExtendedAddress = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 0,
                  address[0], address[1], address[2], address[3],
                  address[4], address[5], address[6], address[7]);
    gpMacDispatcher_SetExtendedAddress(&extendedMac, qorvoGetStackId());
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
    return qorvoToThreadError(res);
}

void qorvoRadioClearSrcMatchEntries(void)
{
    /* clear both short and extended addresses here */
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
