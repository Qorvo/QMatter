/*
 * Copyright (c) 2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_QORVOBLEHOST

#include "gpLog.h"
#include "gpSched.h"

#include "gpHci.h"

#include "cordioBleHost.h"

// ARM Cordio includes
#include "wsf_types.h"
#include "wsf_buf.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "bstream.h"
#include "hci_api.h"
#include "hci_main.h"
#include "hci_evt.h"
#include "hci_cmd.h"
#include "hci_core.h"

#include "gpBle.h"
#include "gpBleConfig.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static Bool BleHost_ForwardLegacyAdvReportEvent(gpHci_LeMetaAdvertisingReportParams_t* pReports, Bool directed);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void cordioBleHost_GetDeviceAddress(BtDeviceAddress_t *bdAddr)
{
    GP_ASSERT_DEV_EXT(bdAddr != NULL);

    gpHci_CommandParameters_t cmd;
    gpBle_EventBuffer_t evt;

    gpBle_ReadBdAddr(&cmd, &evt);

    MEMCPY(bdAddr,
           evt.payload.commandCompleteParams.returnParams.bdAddress.addr,
           sizeof(BtDeviceAddress_t));
}

void cordioBleHost_SetDeviceAddress(const BtDeviceAddress_t *bdAddr)
{
    gpHci_CommandParameters_t cmd;
    gpBle_EventBuffer_t evt;

    MEMCPY(&cmd.VsdWriteDeviceAddress.address, bdAddr,sizeof(BtDeviceAddress_t));


    if(gpBle_VsdWriteDeviceAddress(&cmd, &evt) == gpHci_ResultSuccess)
    {
        /* Also let stack know the new address */
        MEMCPY(hciCoreCb.bdAddr, bdAddr, sizeof(BtDeviceAddress_t));
    }
}

void cordioBleHost_SetTransmitPower(Int8 transmitPower)
{
#ifdef GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
    gpHci_CommandParameters_t cmd;
    cmd.VsdSetTransmitPower.transmitPower = transmitPower;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdSetTransmitPower,gpBle_VsdSetTransmitPower, &cmd);
#endif //GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
}

void cordioBleHost_SetEventNotificationBit(UInt8 event)
{
    gpHci_CommandParameters_t cmd;
    cmd.SetLeMetaVSDEvent.eventCode = event;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeSetLeMetaVSDEvent, gpBle_SetLeMetaVSDEvent, &cmd);
}

void cordioBleHost_ResetEventNotificationBit(UInt8 event)
{
    gpHci_CommandParameters_t cmd;
    cmd.ResetLeMetaVSDEvent.eventCode = event;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeResetLeMetaVSDEvent, gpBle_ResetLeMetaVSDEvent, &cmd);
}

void cordioBleHost_SetVsdDualModeTimeFor15Dot4(UInt32 timeFor15Dot4)
{
    gpHci_CommandParameters_t cmd;
    cmd.SetVsdTestParams.type = gpBle_SetVsdDualModeTimeFor15Dot4Type;
    cmd.SetVsdTestParams.value = (UInt8 *)&timeFor15Dot4;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeSetVsdTestParams,gpBle_SetVsdTestParams, &cmd);
}

static void BleHost_SetEventMask(void)
{
    UInt8 eventMask[8];
    //Enable all events - Meta not default enabled
    MEMSET(eventMask, 0xFF, sizeof(eventMask));
    HciSetEventMaskCmd(eventMask);
}

static void BleHost_LeSetEventMask(void)
{
    HciLeSetEventMaskCmd((uint8_t *) hciLeEventMask);
}

static void BleHost_ResetSequence(gpHci_CommandCompleteParams_t* pBuf)
{
    wsfMsgHdr_t hdr;

    switch (pBuf->opCode)
    {
      case gpHci_OpCodeReset:
        gpSched_ScheduleEvent(0,BleHost_SetEventMask);
        break;

      case gpHci_OpCodeSetEventMask:
        gpSched_ScheduleEvent(0,BleHost_LeSetEventMask);
        break;

      case gpHci_OpCodeLeSetEventMask:
        gpSched_ScheduleEvent(0,HciLeReadBufSizeCmd);
        break;

      case gpHci_OpCodeLeReadBufferSize:
        hciCoreCb.bufSize = pBuf->returnParams.leReadBufferSize.ACLDataPacketLength;
        hciCoreCb.numBufs = pBuf->returnParams.leReadBufferSize.totalNumDataPackets;
        hciCoreCb.maxRxAclLen = hciCoreCb.bufSize;
        hciCoreCb.availBufs = hciCoreCb.numBufs;
        HciReadBdAddrCmd();
        gpSched_ScheduleEvent(0,HciLeReadSupStatesCmd);
        break;

      case gpHci_OpCodeLeReadSupportedStates:
        memcpy(hciCoreCb.leStates,
               pBuf->returnParams.supportedFeatures.supportedFeatures,
               HCI_LE_STATES_LEN);
        gpSched_ScheduleEvent(0,HciLeReadLocalSupFeatCmd);
        break;

      case gpHci_OpCodeLeReadLocalSupportedFeatures:
        memcpy(&hciCoreCb.leSupFeat,
               pBuf->returnParams.supportedFeatures.supportedFeatures,
               sizeof(hciCoreCb.leSupFeat));
        gpSched_ScheduleEvent(0,HciLeReadWhiteListSizeCmd);
        break;

      case gpHci_OpCodeLeReadFilterAcceptListSize:
        hciCoreCb.whiteListSize = pBuf->returnParams.filterAcceptListSize;
        gpSched_ScheduleEvent(0,HciLeReadResolvingListSize);
        break;

      case gpHci_OpCodeLeReadResolvingListSize:
        if(pBuf->result == HCI_SUCCESS)
        {
            hciCoreCb.resListSize =  pBuf->returnParams.resolvingListSize;
        }
        else
        {
            hciCoreCb.resListSize = 0;
        }
        GP_LOG_PRINTF("Resolving list size:%d res:%u", 0, hciCoreCb.resListSize, pBuf->result);
        if (hciCb.secCback)
        {
          uint8_t randCount = 0;
          /* HCI_RESET_RAND_CNT random numbers used to fill the rand buffers in SMP (secCb.rand). */
          while (randCount < HCI_RESET_RAND_CNT)
          {
            randCount++;
            HciLeRandCmd();
          }
        }


        /* last command in sequence; set resetting state and call callback */
        hciCb.resetting = FALSE;
        hdr.param = 0;
        hdr.event = HCI_RESET_SEQ_CMPL_CBACK_EVT;
        (*hciCb.evtCback)((hciEvt_t *) &hdr);
        break;

      default:
        break;
    }
}

Bool BleHost_ForwardLegacyAdvReportEvent(gpHci_LeMetaAdvertisingReportParams_t* pReports, Bool directed)
{
    hciEvt_t evt;
    UIntLoop i;

    MEMSET(&evt, 0, sizeof(hciEvt_t));

    evt.leAdvReport.hdr.param = 0;
    evt.leAdvReport.hdr.event = HCI_LE_ADV_REPORT_CBACK_EVT;
    evt.leAdvReport.hdr.status = HCI_SUCCESS;

    for(i = 0; i <  pReports->nrOfReports; i++)
    {
        gpHci_ReportPayload_t* pReport = &pReports->reports[i];

        evt.leAdvReport.eventType = pReport->eventType;
        evt.leAdvReport.addrType  = pReport->addressType;
        MEMCPY(evt.leAdvReport.addr, &pReport->address, sizeof(evt.leAdvReport.addr));

        if(directed)
        {
            evt.leAdvReport.directAddrType   = pReport->data.directed.directAddressType;
            MEMCPY(&evt.leAdvReport.directAddr[0], &pReport->data.directed.directAddress.addr[0], sizeof(BtDeviceAddress_t));
            evt.leAdvReport.rssi    = pReport->data.directed.rssi;
        }
        else
        {
            evt.leAdvReport.len     = pReport->data.undirected.dataLength;
            evt.leAdvReport.pData   = pReport->data.undirected.data;
            evt.leAdvReport.rssi    = pReport->data.undirected.rssi;
        }

        hciCb.evtCback(&evt);
    }

    return true;
}

/*****************************************************************************
 *                    External Function Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

//Messages to host
Bool WcBleHost_gpHci_CommandCompleteHandler(gpHci_EventCode_t eventCode,gpHci_CommandCompleteParams_t * pBuf)
{
    hciEvt_t evt;

    /* if HCI Reset is initiated then deliver events to Reset sequence. */
    if (hciCb.resetting)
    {
      BleHost_ResetSequence(pBuf);
      return true;
    }

    evt.hdr.event = 0xFF; //Invalid
    evt.hdr.param = 0;
    evt.hdr.status = pBuf->result;


    switch(pBuf->opCode)
    {
        case gpHci_OpCodeLeReadBufferSize:
            {
                hciCoreCb.bufSize = pBuf->returnParams.leReadBufferSize.ACLDataPacketLength;
                hciCoreCb.numBufs = pBuf->returnParams.leReadBufferSize.totalNumDataPackets;
                hciCoreCb.maxRxAclLen = hciCoreCb.bufSize;
                break;
            }

        /* The Reset event is is handled by the Reset sequence, see BleHost_ResetSequence() */
        case gpHci_OpCodeReset:
             break;

        case gpHci_OpCodeReadBdAddr:
            {
                MEMCPY(hciCoreCb.bdAddr, pBuf->returnParams.bdAddress.addr, sizeof(hciCoreCb.bdAddr));
                break;
            }
        case gpHci_OpCodeLeCreateConnectionCancel:
            {
                evt.hdr.event = HCI_LE_CREATE_CONN_CANCEL_CMD_CMPL_CBACK_EVT;
                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeEncrypt:
            {
                //Done synchronous
                evt.hdr.event = HCI_LE_ENCRYPT_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.connectionHandle;

                evt.leEncryptCmdCmpl.status = evt.hdr.status;
                MEMCPY(evt.leEncryptCmdCmpl.data, pBuf->returnParams.encryptedData.encryptedData, HCI_ENCRYPT_DATA_LEN);

                hciCb.secCback(&evt);
                break;
            }
        case gpHci_OpCodeLeLongTermKeyRequestReply:
            {
                evt.hdr.event = HCI_LE_LTK_REQ_REPL_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.connectionHandle;

                evt.leLtkReqReplCmdCmpl.handle = pBuf->returnParams.connectionHandle;
                evt.leLtkReqReplCmdCmpl.status = evt.hdr.status;

                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeLongTermKeyRequestNegativeReply:
            {
                evt.hdr.event = HCI_LE_LTK_REQ_NEG_REPL_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.connectionHandle;

                evt.leLtkReqNegReplCmdCmpl.handle = pBuf->returnParams.connectionHandle;
                evt.leLtkReqNegReplCmdCmpl.status = evt.hdr.status;

                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeRemoteConnectionParamRequestReply:
            {
                evt.hdr.event = HCI_LE_REM_CONN_PARAM_REP_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.connectionHandle;

                evt.leRemConnParamRepCmdCmpl.handle = pBuf->returnParams.connectionHandle;
                evt.leRemConnParamRepCmdCmpl.status = evt.hdr.status;

                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeRemoteConnectionParamRequestNegReply:
            {
                evt.hdr.event = HCI_LE_LTK_REQ_NEG_REPL_CMD_CMPL_CBACK_EVT;

                evt.leRemConnParamNegRepCmdCmpl.handle = pBuf->returnParams.connectionHandle;
                evt.leRemConnParamNegRepCmdCmpl.status = evt.hdr.status;

                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeRand:
            {
                evt.hdr.event = HCI_LE_RAND_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.connectionHandle;
                //Done synchronous
                break;
            }
        case gpHci_OpCodeLeReadChannelMap:
            {
                evt.hdr.event = HCI_LE_READ_CHAN_MAP_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.leReadChannelMap.connectionHandle;
                MEMCPY(evt.readChanMapCmdCmpl.chanMap, pBuf->returnParams.leReadChannelMap.channelMap.channels, sizeof(evt.readChanMapCmdCmpl.chanMap));

                evt.readChanMapCmdCmpl.handle = pBuf->returnParams.leReadChannelMap.connectionHandle;
                evt.readChanMapCmdCmpl.status = evt.hdr.status;

                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeReadRSSI:
            {
                evt.hdr.event = HCI_READ_RSSI_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.connectionHandle;

                evt.readRssiCmdCmpl.handle = pBuf->returnParams.connectionHandle;
                evt.readRssiCmdCmpl.status = evt.hdr.status;

                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeReadTransmitPowerLevel:
            {
                evt.hdr.event = HCI_READ_TX_PWR_LVL_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.connectionHandle;

                evt.readTxPwrLvlCmdCmpl.handle = pBuf->returnParams.connectionHandle;
                evt.readTxPwrLvlCmdCmpl.status = evt.hdr.status;

                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeReadPhy:
            {

                evt.hdr.event = HCI_LE_READ_PHY_CMD_CMPL_CBACK_EVT;
                evt.hdr.param = pBuf->returnParams.leReadPhy.connectionHandle;

                evt.leReadPhyCmdCmpl.handle = pBuf->returnParams.leReadPhy.connectionHandle;
                evt.leReadPhyCmdCmpl.status = evt.hdr.status;

                evt.leReadPhyCmdCmpl.txPhy = pBuf->returnParams.leReadPhy.txPhy;
                evt.leReadPhyCmdCmpl.rxPhy = pBuf->returnParams.leReadPhy.rxPhy;


                hciCb.evtCback(&evt);

                break;
            }
        case gpHci_OpCodeLeSetAdvertiseEnable:
            {
                evt.hdr.event = HCI_LE_ADV_ENABLE_CMD_CMPL_CBACK_EVT;

                hciCb.evtCback(&evt);

                break;
            }
        case gpHci_OpCodeLeSetScanEnable:
            {
                evt.hdr.event = HCI_LE_SCAN_ENABLE_CMD_CMPL_CBACK_EVT;

                hciCb.evtCback(&evt);

                break;
            }
        case gpHci_OpCodeLeAddDeviceToResolvingList:
            {
                evt.hdr.event = HCI_LE_ADD_DEV_TO_RES_LIST_CMD_CMPL_CBACK_EVT;
                evt.leAddDevToResListCmdCmpl.status = evt.hdr.status;
                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeRemoveDeviceFromResolvingList:
            {
                evt.hdr.event = HCI_LE_REM_DEV_FROM_RES_LIST_CMD_CMPL_CBACK_EVT;
                evt.leRemDevFromResListCmdCmpl.status = evt.hdr.status;
                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeClearResolvingList:
            {
                evt.hdr.event = HCI_LE_CLEAR_RES_LIST_CMD_CMPL_CBACK_EVT;
                evt.leClearResListCmdCmpl.status = evt.hdr.status;
                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeVsdGeneratePeerResolvableAddress:
            {
                evt.hdr.event = HCI_LE_READ_PEER_RES_ADDR_CMD_CMPL_CBACK_EVT;
                evt.leReadPeerResAddrCmdCmpl.status = evt.hdr.status;
                MEMCPY(evt.leReadPeerResAddrCmdCmpl.peerRpa, &pBuf->returnParams.bdAddress, BDA_ADDR_LEN);
                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeVsdGenerateLocalResolvableAddress:
            {
                evt.hdr.event = HCI_LE_READ_LOCAL_RES_ADDR_CMD_CMPL_CBACK_EVT;
                evt.leReadLocalResAddrCmdCmpl.status = evt.hdr.status;
                MEMCPY(evt.leReadLocalResAddrCmdCmpl.localRpa, &pBuf->returnParams.bdAddress, BDA_ADDR_LEN);
                hciCb.evtCback(&evt);
                break;
            }
        case gpHci_OpCodeLeSetAddressResolutionEnable:
            {
                evt.hdr.event = HCI_LE_SET_ADDR_RES_ENABLE_CMD_CMPL_CBACK_EVT;
                evt.leSetAddrResEnableCmdCmpl.status = evt.hdr.status;
                hciCb.evtCback(&evt);
                break;
            }



#if defined(GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED)
        case gpHci_OpCodeVsdSetScanCoexParams:
#endif // GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED
        case gpHci_OpCodeLeSetDataLength:
        case gpHci_OpCodeLeSetScanResponseData:
        case gpHci_OpCodeSetEventMask:
        case gpHci_OpCodeLeSetEventMask:
        case gpHci_OpCodeVsdWriteDeviceAddress:
        case gpHci_OpCodeLeSetAdvertisingParameters:
        case gpHci_OpCodeLeSetAdvertisingData:
        case gpHci_OpCodeLeSetScanParameters:
        case gpHci_OpCodeVsdDisableSlaveLatency:
        case gpHci_OpCodeVsdEnCBByDefault:
        case gpHci_OpCodeVsdOverruleRemoteMaxRxOctetsAndTime:
        case gpHci_OpCodeVsdSetTransmitPower:
        case gpHci_OpCodeLeSetRandomAddress:
        case gpHci_OpcodeNoOperation:
        case gpHci_OpCodeSetVsdTestParams:
        case gpHci_OpCodeLeSetDefaultPhy:
        case gpHci_OpCodeLeClearFilterAcceptList:
        case gpHci_OpCodeLeAddDeviceToFilterAcceptList:
        case gpHci_OpCodeLeRemoveDeviceFromFilterAcceptList:
        case gpHci_OpCodeLeReadResolvingListSize:
        case gpHci_OpCodeLeSetResolvablePrivateAddressTimeout:
        case gpHci_OpCodeLeSetPrivacyMode:
        case gpHci_OpCodeLeReadLocalSupportedFeatures:
        case gpHci_OpCodeReadLocalVersionInformation:
        case gpHci_OpCodeLeSetAdvertisingSetRandomAddress:
        case gpHci_OpCodeLeSetExtendedAdvertisingParameters:
        case gpHci_OpCodeLeSetExtendedAdvertisingData:
        case gpHci_OpCodeLeSetExtendedScanResponseData:
        case gpHci_OpCodeLeRemoveAdvertisingSet:
        case gpHci_OpCodeLeClearAdvertisingSets:
        case gpHci_OpCodeLeSetPeriodicAdvertisingParameters:
        case gpHci_OpCodeLeSetPeriodicAdvertisingData:
        case gpHci_OpCodeLeSetExtendedScanParameters:
        case gpHci_OpCodeLeExtendedCreateConnection:
        case gpHci_OpCodeLePeriodicAdvertisingCreateSync:
        case gpHci_OpCodeLePeriodicAdvertisingCreateSyncCancel:
        case gpHci_OpCodeLePeriodicAdvertisingTerminateSync:
        case gpHci_OpCodeLeAddDeviceToPeriodicAdvertiserList:
        case gpHci_OpCodeLeRemoveDeviceFromPeriodicAdvertiserList:
        case gpHci_OpCodeLeClearPeriodicAdvertiserList:
             {
                // These Command Complete Events are not needed for the stack
                if(pBuf->result != HCI_SUCCESS)
                {
                    GP_LOG_SYSTEM_PRINTF("HCI command complete event %x returned error %d",0,pBuf->opCode, pBuf->result);
                    GP_ASSERT_DEV_EXT(false);
                }
                if (hciCb.unhandledCmdComplEvtCback)
                {
                    hciCb.unhandledCmdComplEvtCback(pBuf->opCode, pBuf->result, &pBuf->returnParams);
                }
                break;
            }
        default:
            {
                GP_LOG_SYSTEM_PRINTF("Unhandled Cmd:%x",0,pBuf->opCode);
                // We should not have unhandled Cmd's,
                GP_ASSERT_SYSTEM(false);
                break;
            }
    }

    return true;
}

Bool gpHci_SendHciDataFrameToHost(UInt16 connAndBoundary,UInt16 dataLength,UInt8 * pData)
{
    UInt8* pMsg;
    UInt16 aclLength = dataLength-4;  /* Datalength is actually the hci packet length*/

    //Using wsf malloc mechanism - buffer will be freed after handling
#ifndef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
    pMsg = WsfMsgAlloc(sizeof(connAndBoundary) + sizeof(aclLength) + (aclLength));
#else
    pMsg = pData;
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
    if(pMsg != NULL)
    {
        //Buffer expected - handle+boundary flags | ACL length | ACL data
        MEMCPY(&pMsg[0], &connAndBoundary, sizeof(connAndBoundary));
        MEMCPY(&pMsg[2], &aclLength , sizeof(aclLength));
#ifndef GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY
        //4 bytes allocated for HCI header - need to be skipped and corrected here
        MEMCPY(&pMsg[4], &pData[4], aclLength);
#endif /* GP_BLE_DIVERSITY_OPTIMIZE_MEMCPY */
        //Signal Host stack
        WsfMsgEnq(&hciCb.rxQueue, HCI_ACL_TYPE, pMsg);
        WsfSetEvent(hciCb.handlerId, HCI_EVT_RX);

        return true;
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("No buffer!",0);
        return false;
    }
}

Bool gpHci_HardwareErrorEvent(UInt8 eventCode,UInt8 hwcode)
{
    hciEvt_t evt;

    evt.hdr.param = 0;
    evt.hdr.event = HCI_HW_ERROR_CBACK_EVT;
    evt.hdr.status = HCI_SUCCESS;

    evt.hwError.code = hwcode;

    hciCb.evtCback(&evt);

    return true;
}

Bool gpHci_DisconnectionCompleteEvent(UInt8 eventCode,gpHci_DisconnectCompleteParams_t * disconnectComplete)
{
    hciEvt_t evt;

    evt.disconnectCmpl.hdr.param = disconnectComplete->connectionHandle; //?
    evt.disconnectCmpl.hdr.event = HCI_DISCONNECT_CMPL_CBACK_EVT;
    evt.disconnectCmpl.hdr.status = disconnectComplete->status; //?

    evt.disconnectCmpl.handle = disconnectComplete->connectionHandle;
    evt.disconnectCmpl.status = disconnectComplete->status;
    evt.disconnectCmpl.reason = disconnectComplete->reason;

    hciCoreConnClose(disconnectComplete->connectionHandle);

    hciCb.evtCback(&evt);

    return true;
}

Bool gpHci_NumberOfCompletedPacketsEvent(UInt8 eventCode,gpHci_NumberOfCompletedPackets_t * numberOfCompletedPackets)
{
    hciCoreNumCmplPkts(numberOfCompletedPackets->handle, numberOfCompletedPackets->nrOfHciPackets);

    return true;
}

Bool gpHci_VsdSinkRxIndication(UInt8 eventCode, gpHci_VsdSinkRxIndication_t* vsdSinkRxIndication)
{
    // unsupported
    return true;
}

Bool gpHci_EncryptionKeyRefreshCompleteEvent(UInt8 eventCode,gpHci_EncryptionKeyRefreshComplete_t * keyRefresh)
{
    hciEvt_t evt;

    evt.encKeyRefreshCmpl.hdr.param  = keyRefresh->connectionHandle;
    evt.encKeyRefreshCmpl.hdr.event  = HCI_ENC_KEY_REFRESH_CMPL_CBACK_EVT;
    evt.encKeyRefreshCmpl.hdr.status = keyRefresh->status;

    evt.encKeyRefreshCmpl.handle = keyRefresh->connectionHandle;
    evt.encKeyRefreshCmpl.status = keyRefresh->status;

    hciCb.evtCback(&evt);

    return true;
}

Bool gpHci_CommandStatusEvent(gpHci_EventCode_t eventCode, gpHci_CommandStatusParams_t* commandStatusParams)
{
    return true;
}
Bool gpHci_LEEnhancedConnectionCompleteEvent(gpHci_LEEnhancedConnectionCompleteEventParams_t* LEEnhancedConnectionComplete)
{
    hciEvt_t evt;

    evt.leConnCmpl.hdr.param = LEEnhancedConnectionComplete->connectionHandle;
    evt.leConnCmpl.hdr.event = HCI_LE_ENHANCED_CONN_CMPL_CBACK_EVT;
    evt.leConnCmpl.hdr.status = LEEnhancedConnectionComplete->status;

    evt.leConnCmpl.status = LEEnhancedConnectionComplete->status;
    evt.leConnCmpl.handle = LEEnhancedConnectionComplete->connectionHandle;
    evt.leConnCmpl.role = LEEnhancedConnectionComplete->role;
    evt.leConnCmpl.addrType = LEEnhancedConnectionComplete->peerAddressType;
    MEMCPY(&evt.leConnCmpl.peerAddr, &LEEnhancedConnectionComplete->peerAddress, BDA_ADDR_LEN);
    evt.leConnCmpl.connInterval = LEEnhancedConnectionComplete->connInterval;
    evt.leConnCmpl.connLatency = LEEnhancedConnectionComplete->connLatency;
    evt.leConnCmpl.supTimeout = LEEnhancedConnectionComplete->supervisionTo;
    evt.leConnCmpl.clockAccuracy = LEEnhancedConnectionComplete->masterClockAccuracy;

    MEMCPY(&evt.leConnCmpl.localRpa, &LEEnhancedConnectionComplete->localPrivateAddress, BDA_ADDR_LEN);
    MEMCPY(&evt.leConnCmpl.peerRpa, &LEEnhancedConnectionComplete->peerPrivateAddress, BDA_ADDR_LEN);

    /* In case of directed advertising, an Enhanced Connection Complete Event with failure status can be received when no Master responds */
    if(LEEnhancedConnectionComplete->status == HCI_SUCCESS)
    {
        hciCoreConnOpen(evt.leConnCmpl.handle);
    }

    hciCb.evtCback(&evt);

    return true;
}

Bool gpHci_LEConnectionCompleteEvent(gpHci_LEConnectionCompleteEventParams_t * LEConnectionCompleteEventParams)
{
    hciEvt_t evt;

    evt.leConnCmpl.hdr.param = LEConnectionCompleteEventParams->connectionHandle; //?
    evt.leConnCmpl.hdr.event = HCI_LE_CONN_CMPL_CBACK_EVT;
    evt.leConnCmpl.hdr.status = LEConnectionCompleteEventParams->status; //?

    evt.leConnCmpl.status   = LEConnectionCompleteEventParams->status;
    evt.leConnCmpl.handle   = LEConnectionCompleteEventParams->connectionHandle;
    evt.leConnCmpl.role     = LEConnectionCompleteEventParams->role;
    evt.leConnCmpl.addrType = LEConnectionCompleteEventParams->peerAddressType;
    MEMCPY(evt.leConnCmpl.peerAddr, LEConnectionCompleteEventParams->peerAddress.addr, sizeof(LEConnectionCompleteEventParams->peerAddress));
    evt.leConnCmpl.connInterval  = LEConnectionCompleteEventParams->connInterval;
    evt.leConnCmpl.connLatency   = LEConnectionCompleteEventParams->connLatency;
    evt.leConnCmpl.supTimeout    = LEConnectionCompleteEventParams->supervisionTo;
    evt.leConnCmpl.clockAccuracy = LEConnectionCompleteEventParams->masterClockAccuracy;

    /* LEConnectionCompleteEventParams do not have RPA fields */
    memset(evt.leConnCmpl.localRpa,0, BDA_ADDR_LEN);
    memset(evt.leConnCmpl.peerRpa,0, BDA_ADDR_LEN);

    /* In case of directed advertising, a Connection Complete Event with failure status can be received when no Master responds */
    if(LEConnectionCompleteEventParams->status == HCI_SUCCESS)
    {
        hciCoreConnOpen(evt.leConnCmpl.handle);
    }

    hciCb.evtCback(&evt);

    return true;
}

Bool gpHci_LEAdvertisingReportEvent(gpHci_LeMetaAdvertisingReportParams_t* pReports)
{
    return BleHost_ForwardLegacyAdvReportEvent(pReports, false);
}

Bool gpHci_LEDataLengthChangeEvent(gpHci_LeMetaDataLengthChange_t * LEDataLengthChangeEvent)
{
    hciEvt_t evt;

    evt.leDataLenChange.hdr.param = LEDataLengthChangeEvent->connectionHandle;
    evt.leDataLenChange.hdr.event = HCI_LE_DATA_LEN_CHANGE_CBACK_EVT;
    /* evt.leDataLenChange.hdr.status = LEDataLengthChangeEvent->status; */


    evt.leDataLenChange.handle = LEDataLengthChangeEvent->connectionHandle;
    evt.leDataLenChange.maxTxOctets = LEDataLengthChangeEvent->MaxTxOctets;
    evt.leDataLenChange.maxTxTime = LEDataLengthChangeEvent->MaxTxTime;
    evt.leDataLenChange.maxRxOctets = LEDataLengthChangeEvent->MaxRxOctets;
    evt.leDataLenChange.maxRxTime = LEDataLengthChangeEvent->MaxRxTime;

    hciCb.evtCback(&evt);

    return true;
}

Bool gpHci_LEConnectionUpdateCompleteEvent(gpHci_LEConnectionUpdateCompleteEventParams_t * LEConnectionUpdateCompleteEvent)
{
    hciEvt_t evt;

    GP_LOG_PRINTF("%u> connection update complete!",0,__LINE__);

    evt.leConnUpdateCmpl.hdr.param  = LEConnectionUpdateCompleteEvent->connectionHandle;
    evt.leConnUpdateCmpl.hdr.event  = HCI_LE_CONN_UPDATE_CMPL_CBACK_EVT;
    evt.leConnUpdateCmpl.hdr.status = LEConnectionUpdateCompleteEvent->status;

    evt.leConnUpdateCmpl.status       = LEConnectionUpdateCompleteEvent->status;
    evt.leConnUpdateCmpl.handle       = LEConnectionUpdateCompleteEvent->connectionHandle;
    evt.leConnUpdateCmpl.connInterval = LEConnectionUpdateCompleteEvent->connInterval;
    evt.leConnUpdateCmpl.connLatency  = LEConnectionUpdateCompleteEvent->connLatency;
    evt.leConnUpdateCmpl.supTimeout   = LEConnectionUpdateCompleteEvent->supervisionTo;

    hciCb.evtCback(&evt);

    return true;
}

Bool gpHci_LEReadRemoteFeaturesCompleteEvent(gpHci_LEReadRemoteFeaturesCompleteParams_t * LEReadRemoteFeaturesComplete)
{
    hciEvt_t evt;

    evt.leReadRemoteFeatCmpl.hdr.param = LEReadRemoteFeaturesComplete->connectionHandle;
    evt.leReadRemoteFeatCmpl.hdr.event = HCI_LE_READ_REMOTE_FEAT_CMPL_CBACK_EVT;
    evt.leReadRemoteFeatCmpl.hdr.status = LEReadRemoteFeaturesComplete->status;

    evt.leReadRemoteFeatCmpl.handle = LEReadRemoteFeaturesComplete->connectionHandle;
    evt.leReadRemoteFeatCmpl.status = LEReadRemoteFeaturesComplete->status;
    MEMCPY(evt.leReadRemoteFeatCmpl.features, LEReadRemoteFeaturesComplete->features, sizeof(evt.leReadRemoteFeatCmpl.features));

    GP_LOG_PRINTF("Remote Feature BitMap %0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x", 2,
         evt.leReadRemoteFeatCmpl.features[0],
         evt.leReadRemoteFeatCmpl.features[1],
         evt.leReadRemoteFeatCmpl.features[2],
         evt.leReadRemoteFeatCmpl.features[3],
         evt.leReadRemoteFeatCmpl.features[4],
         evt.leReadRemoteFeatCmpl.features[5],
         evt.leReadRemoteFeatCmpl.features[6],
         evt.leReadRemoteFeatCmpl.features[7]);

    hciCb.evtCback(&evt);

    return true;
}
Bool gpHci_LELongTermKeyRequestEvent(gpHci_LELongTermKeyRequestParams_t * LELongTermKeyRequestEvent)
{
    hciEvt_t evt;

    evt.leLtkReq.hdr.param  = LELongTermKeyRequestEvent->connectionHandle;
    evt.leLtkReq.hdr.event  = HCI_LE_LTK_REQ_CBACK_EVT;
    evt.leLtkReq.hdr.status = HCI_SUCCESS;

    evt.leLtkReq.handle         = LELongTermKeyRequestEvent->connectionHandle;
    evt.leLtkReq.encDiversifier = LELongTermKeyRequestEvent->encryptedDiversifier;
    MEMCPY(evt.leLtkReq.randNum, LELongTermKeyRequestEvent->randomNumber, sizeof(evt.leLtkReq.randNum));

    hciCb.evtCback(&evt);

    return true;
}
Bool gpHci_LERemoteConnectionParameterRequest(gpHci_LERemoteConnectionParamsEventParams_t * LERemoteConnectionParameterRequest)
{
    hciEvt_t evt;

    evt.leRemConnParamReq.hdr.param = LERemoteConnectionParameterRequest->connectionHandle;
    evt.leRemConnParamReq.hdr.event = HCI_LE_REM_CONN_PARAM_REQ_CBACK_EVT;
    evt.leRemConnParamReq.hdr.status = HCI_SUCCESS;


    evt.leRemConnParamReq.handle = LERemoteConnectionParameterRequest->connectionHandle;
    evt.leRemConnParamReq.intervalMin = LERemoteConnectionParameterRequest->connIntervalMin;
    evt.leRemConnParamReq.intervalMax = LERemoteConnectionParameterRequest->connIntervalMax;
    evt.leRemConnParamReq.latency = LERemoteConnectionParameterRequest->connLatency;
    evt.leRemConnParamReq.timeout = LERemoteConnectionParameterRequest->supervisionTimeout;

    hciCb.evtCback(&evt);

    return true;
}

Bool gpHci_LEDirectAdvertisingReportEvent(gpHci_LeMetaAdvertisingReportParams_t* pReports)
{
    return BleHost_ForwardLegacyAdvReportEvent(pReports, true);
}

Bool gpHci_EncryptionChangeEvent(UInt8 eventCode,gpHci_EncryptionChangeParams_t * encryptionChange)
{
    hciEvt_t evt;

    evt.encChange.hdr.param  = encryptionChange->connectionHandle;
    evt.encChange.hdr.event  = HCI_ENC_CHANGE_CBACK_EVT;
    evt.encChange.hdr.status = encryptionChange->status;

    evt.encChange.status  = encryptionChange->status;
    evt.encChange.handle  = encryptionChange->connectionHandle;
    evt.encChange.enabled = encryptionChange->encryptionEnabled;

    hciCb.evtCback(&evt);

    return true;
}
Bool gpHci_AuthenticationPayloadTOEvent(UInt8 eventCode,gpHci_AuthenticatedPayloadToExpired_t * authenticatedPayloadToExpired)
{
    hciEvt_t evt;

    evt.authPayloadToExpired.hdr.param = authenticatedPayloadToExpired->connectionHandle;
    evt.authPayloadToExpired.hdr.event = HCI_AUTH_PAYLOAD_TO_EXPIRED_CBACK_EVT;
    evt.authPayloadToExpired.handle = authenticatedPayloadToExpired->connectionHandle;

    hciCb.evtCback(&evt);

    return true;
}
Bool gpHci_DataBufferOverflowEvent(UInt8 eventCode,gpHci_LinkType_t linktype)
{
    return true;
}
Bool gpHci_ReadRemoteVersionCompleteEvent(UInt8 eventCode,gpHci_ReadRemoteVersionInfoComplete_t * versionInfo)
{
    hciEvt_t evt;

    evt.readRemoteVerInfoCmpl.hdr.param  = versionInfo->connectionHandle;
    evt.readRemoteVerInfoCmpl.hdr.event  = HCI_READ_REMOTE_VER_INFO_CMPL_CBACK_EVT;
    evt.readRemoteVerInfoCmpl.hdr.status = versionInfo->status;

    evt.readRemoteVerInfoCmpl.status  = versionInfo->status;
    evt.readRemoteVerInfoCmpl.handle  = versionInfo->connectionHandle;
    evt.readRemoteVerInfoCmpl.mfrName = versionInfo->companyId;
    evt.readRemoteVerInfoCmpl.version = versionInfo->versionNr;
    evt.readRemoteVerInfoCmpl.subversion = versionInfo->subVersionNr;

    hciCb.evtCback(&evt);

    return true;
}
Bool gpHci_ConnectionCompleteEvent(UInt8 eventCode,gpHci_ConnectionCompleteParams_t * connectionComplete)
{
    return true;
}

Bool gpHci_LEPhyUpdateComplete(gpHci_LEPhyUpdateCompleteEventParams_t* LEPhyUpdateComplete)
{
    hciEvt_t evt;

    evt.lePhyUpdate.hdr.param = LEPhyUpdateComplete->connectionHandle;
    evt.lePhyUpdate.hdr.event = HCI_LE_PHY_UPDATE_CMPL_CBACK_EVT;
    evt.lePhyUpdate.hdr.status = LEPhyUpdateComplete->status;

    evt.lePhyUpdate.status = LEPhyUpdateComplete->status;
    evt.lePhyUpdate.handle = LEPhyUpdateComplete->connectionHandle;
    evt.lePhyUpdate.txPhy = LEPhyUpdateComplete->txPhy;
    evt.lePhyUpdate.rxPhy = LEPhyUpdateComplete->rxPhy;

    hciCb.evtCback(&evt);

    return true;
}




#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
Bool gpHci_LeConnectionlessIqReport(gpHci_LEConnectionlessIqReportEventParams_t* LeConnectionlessIqReport)
{
    GP_LOG_SYSTEM_PRINTF("gpHci_LeConnectionlessIqReport not supported",0);
    GP_ASSERT_SYSTEM(false);
    return true;
}

Bool gpHci_LeConnectionIqReport(gpHci_LEConnectionIqReportEventParams_t* LeConnectionIqReport)
{
    return true;
}

Bool gpHci_LeCteRequestFailed(gpHci_LECteRequestFailedEventParams_t* LeCteRequestFailed)
{
    return true;
}
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */

Bool gpHci_VsdMetaFilterAcceptListModifiedEvent(gpHci_VsdMetaFilterAcceptListModified_t* filterAcceptListModifiedParams)
{
    // GP_ASSERT_SYSTEM(false); // Needs implementation
    return false; //(more friendly )  Needs implementation
}
