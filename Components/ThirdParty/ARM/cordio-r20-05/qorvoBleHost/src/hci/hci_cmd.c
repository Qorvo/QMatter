/*!
 *  \file   hci_cmd.c
 *
 *  \brief  HCI command module.
 *
 *  Copyright (c) 2018 Arm Ltd. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/*************************************************************************************************/


#include <string.h>
#include "wsf_types.h"
#include "wsf_queue.h"
#include "wsf_timer.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "bstream.h"
#include "hci_cmd.h"
#include "hci_tr.h"
#include "hci_api.h"
#include "hci_main.h"
#include "hci_core.h"

#if defined(GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED) 
#include "gpHal_Coex.h"
#endif // defined(GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED) || defined(GP_DIVERSITY_BLE_INIT_COEX_SUPPORTED)
#include "gpBle.h"
#include "gpBleConfig.h"
#include "gpBleDataCommon.h"
#include "gpBleSecurityCoprocessor.h"
#include "gpBleAddressResolver.h"

#if defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_LEGACY_ADVERTISING)
#include "gpBleAdvertiser.h"
#endif //GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_PERIPHERAL

#if defined(GP_DIVERSITY_BLE_OBSERVER) || defined (GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED)
#include "gpBleScanner.h"
#endif //GP_DIVERSITY_BLE_OBSERVER || GP_DIVERSITY_BLE_CENTRAL

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleInitiator.h"
#include "gpBleLlcpProcedures.h"
#include "gpBleLlcp.h"
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

#ifdef GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED
#include "gpBleConnectionManager.h"
#endif //GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
#include "gpBleResPrAddr.h"
#endif

extern hciCoreCb_t hciCoreCb;


/*************************************************************************************************/
/*!
 *  \fn     HciDisconnectCmd
 *
 *  \brief  HCI disconnect command.
 */
/*************************************************************************************************/
void HciDisconnectCmd(uint16_t handle, uint8_t reason)
{
    gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED
    cmd.Disconnect.connectionHandle = handle;
    cmd.Disconnect.reason = reason;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeDisconnect, gpBle_Disconnect, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeDisconnect, gpBle_UnknownOpCode, &cmd);
#endif //GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeAddDevWhiteListCmd
 *
 *  \brief  HCI LE add device white list command.
 */
/*************************************************************************************************/
void HciLeAddDevWhiteListCmd(uint8_t addrType, uint8_t *pAddr)
{
    gpHci_CommandParameters_t cmd;

    cmd.LeAddDeviceToFilterAcceptList.addressType = addrType;
    MEMCPY(&cmd.LeAddDeviceToFilterAcceptList.address.addr, pAddr, sizeof(cmd.LeAddDeviceToFilterAcceptList.address.addr));

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeAddDeviceToFilterAcceptList, gpBle_LeAddDeviceToFilterAcceptList, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeClearWhiteListCmd
 *
 *  \brief  HCI LE clear white list command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeClearWhiteListCmd(void)
{
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeClearFilterAcceptList, gpBle_LeClearFilterAcceptList, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeConnUpdateCmd
 *
 *  \brief  HCI connection update command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeConnUpdateCmd(uint16_t handle, hciConnSpec_t *pConnSpec)
{
    gpHci_CommandParameters_t cmd;

    cmd.LeConnectionUpdate.connectionHandle = handle;

    cmd.LeConnectionUpdate.connIntervalMin = pConnSpec->connIntervalMin;
    cmd.LeConnectionUpdate.connIntervalMax = pConnSpec->connIntervalMax;
    cmd.LeConnectionUpdate.connLatency = pConnSpec->connLatency;
    cmd.LeConnectionUpdate.supervisionTimeout = pConnSpec->supTimeout;
    cmd.LeConnectionUpdate.minCELength = pConnSpec->minCeLen;
    cmd.LeConnectionUpdate.maxCELength = pConnSpec->maxCeLen;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeConnectionUpdate, gpBle_LeConnectionUpdate, &cmd);
}


/*************************************************************************************************/
/*!
 *  \fn     HciLeRandCmd
 *
 *  \brief  HCI LE random command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeRandCmd(void)
{
    hciEvt_t evt;
    gpBle_EventBuffer_t evtCmd;

    evt.hdr.param = 0;
    evt.hdr.event = HCI_LE_RAND_CMD_CMPL_CBACK_EVT;
#ifdef GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED
    evt.hdr.status = gpBle_LeRand(NULL, &evtCmd);

    evt.leRandCmdCmpl.status = evt.hdr.status;

    MEMCPY(evt.leRandCmdCmpl.randNum,
           evtCmd.payload.commandCompleteParams.returnParams.randData.randomNumber,
           sizeof(evt.leRandCmdCmpl.randNum));
#else
    // Add unsupported callback
    evt.hdr.status = HCI_ERR_UNKNOWN_CMD;
    evt.leRandCmdCmpl.status = evt.hdr.status;

    NOT_USED(evtCmd);
#endif //GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED
    hciCb.secCback(&evt);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeReadAdvTXPowerCmd
 *
 *  \brief  HCI LE read advertising TX power command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadAdvTXPowerCmd(void)
{
  /* unused */
    gpHci_CommandParameters_t cmd;
#if defined(GP_DIVERSITY_BLE_LEGACY_ADVERTISING)
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadAdvertisingChannelTxPower, gpBle_LeReadAdvertisingChannelTxPower, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadAdvertisingChannelTxPower, gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeReadBufSizeCmd
 *
 *  \brief  HCI LE read buffer size command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadBufSizeCmd(void)
{
  /* unused */
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadBufferSize, gpBle_LeReadBufferSize, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeReadChanMapCmd
 *
 *  \brief  HCI LE read channel map command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadChanMapCmd(uint16_t handle)
{
    gpHci_CommandParameters_t cmd;

    cmd.LeReadChannelMap.connectionHandle = handle;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadChannelMap, gpBle_LeReadChannelMap, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeReadLocalSupFeatCmd
 *
 *  \brief  HCI LE read local supported feautre command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadLocalSupFeatCmd(void)
{
    gpHci_CommandParameters_t cmd;
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadLocalSupportedFeatures, gpBle_LeReadLocalSupportedFeatures, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeReadRemoteFeatCmd
 *
 *  \brief  HCI LE read remote feature command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadRemoteFeatCmd(uint16_t handle)
{
    gpHci_CommandParameters_t cmd;

    cmd.LeReadRemoteFeatures.connectionHandle = handle;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadRemoteFeatures, gpBle_LeReadRemoteFeatures, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeReadSupStatesCmd
 *
 *  \brief  HCI LE read supported states command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadSupStatesCmd(void)
{
  /* unsupported */
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadSupportedStates, gpBle_LeReadSupportedStates, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeReadWhiteListSizeCmd
 *
 *  \brief  HCI LE read white list size command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadWhiteListSizeCmd(void)
{
  /* unused */
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadFilterAcceptListSize, gpBle_LeReadFilterAcceptListSize, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeRemoveDevWhiteListCmd
 *
 *  \brief  HCI LE remove device white list command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeRemoveDevWhiteListCmd(uint8_t addrType, uint8_t *pAddr)
{
    gpHci_CommandParameters_t cmd;

    cmd.LeRemoveDeviceFromFilterAcceptList.addressType = addrType;
    MEMCPY(&cmd.LeRemoveDeviceFromFilterAcceptList.address, pAddr, sizeof(cmd.LeCreateConnection.peerAddress));

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeRemoveDeviceFromFilterAcceptList, gpBle_LeRemoveDeviceFromFilterAcceptList, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeSetAdvEnableCmd
 *
 *  \brief  HCI LE set advanced enable command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeSetAdvEnableCmd(uint8_t enable)
{
    gpHci_CommandParameters_t cmd;
#if defined(GP_DIVERSITY_BLE_LEGACY_ADVERTISING)
    cmd.LeSetAdvertiseEnable.enable = enable;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetAdvertiseEnable, gpBle_LeSetAdvertiseEnable, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetAdvertiseEnable, gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeSetAdvDataCmd
 *
 *  \brief  HCI LE set advertising data command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeSetAdvDataCmd(uint8_t len, uint8_t *pData)
{
    gpHci_CommandParameters_t cmd;
#if defined(GP_DIVERSITY_BLE_LEGACY_ADVERTISING)
    cmd.LeSetAdvertisingData.length = len;
    cmd.LeSetAdvertisingData.data = pData;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetAdvertisingData, gpBle_LeSetAdvertisingData, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetAdvertisingData, gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeSetAdvParamCmd
 *
 *  \brief  HCI LE set advertising parameters command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeSetAdvParamCmd(uint16_t advIntervalMin, uint16_t advIntervalMax, uint8_t advType,
                         uint8_t ownAddrType, uint8_t peerAddrType, uint8_t *pPeerAddr,
                         uint8_t advChanMap, uint8_t advFiltPolicy)
{
    gpHci_CommandParameters_t cmd;
#if defined(GP_DIVERSITY_BLE_LEGACY_ADVERTISING)
    cmd.LeSetAdvertisingParameters.advertisingIntervalMin = advIntervalMin;
    cmd.LeSetAdvertisingParameters.advertisingIntervalMax = advIntervalMax;
    cmd.LeSetAdvertisingParameters.advertisingType = advType;
    cmd.LeSetAdvertisingParameters.ownAddressType = ownAddrType;
    MEMCPY(&cmd.LeSetAdvertisingParameters.peerAddress.addr, pPeerAddr, sizeof(cmd.LeSetAdvertisingParameters.peerAddress.addr));
    cmd.LeSetAdvertisingParameters.peerAddressType = peerAddrType;
    cmd.LeSetAdvertisingParameters.filterPolicy = advFiltPolicy;
    cmd.LeSetAdvertisingParameters.channelMap = advChanMap;
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetAdvertisingParameters, gpBle_LeSetAdvertisingParameters, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetAdvertisingParameters, gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeSetEventMaskCmd
 *
 *  \brief  HCI LE set event mask command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeSetEventMaskCmd(uint8_t *pLeEventMask)
{
    gpHci_CommandParameters_t cmd;

    MEMCPY(&cmd.LeSetEventMask, pLeEventMask, sizeof(cmd.LeSetEventMask));

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetEventMask, gpBle_LeSetEventMask, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeSetHostChanClassCmd
 *
 *  \brief  HCI set host channel class command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeSetHostChanClassCmd(uint8_t *pChanMap)
{
    gpHci_CommandParameters_t cmd;
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetHostChannelClassification, gpBle_UnknownOpCode, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeSetRandAddrCmd
 *
 *  \brief  HCI LE set random address command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeSetRandAddrCmd(uint8_t *pAddr)
{
    gpHci_CommandParameters_t cmd;

    MEMCPY(cmd.LeSetRandomAddress.address.addr, pAddr, sizeof(cmd.LeSetRandomAddress.address.addr));

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetRandomAddress, gpBle_LeSetRandomAddress, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeSetScanRespDataCmd
 *
 *  \brief  HCI LE set scan response data.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeSetScanRespDataCmd(uint8_t len, uint8_t *pData)
{
    gpHci_CommandParameters_t cmd;
#if defined(GP_DIVERSITY_BLE_LEGACY_ADVERTISING)
    cmd.LeSetScanResponseData.length = len;
    cmd.LeSetScanResponseData.data = pData;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetScanResponseData, gpBle_LeSetScanResponseData, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetScanResponseData, gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     HciReadBdAddrCmd
 *
 *  \brief  HCI read BD address command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciReadBdAddrCmd(void)
{
    gpHci_CommandParameters_t cmd;
    gpBle_EventBuffer_t evt;

    gpBle_ReadBdAddr(&cmd, &evt);

    MEMCPY(hciCoreCb.bdAddr,
           evt.payload.commandCompleteParams.returnParams.bdAddress.addr,
           sizeof(hciCoreCb.bdAddr));
}

/*************************************************************************************************/
/*!
 *  \fn     HciReadBufSizeCmd
 *
 *  \brief  HCI read buffer size command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciReadBufSizeCmd(void)
{
  /* not used */
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeReadBufferSize, gpBle_ReadBufferSize, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciReadLocalSupFeatCmd
 *
 *  \brief  HCI read local supported feature command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciReadLocalSupFeatCmd(void)
{
  /* not used */
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeReadLocalSupportedFeatures, gpBle_ReadLocalSupportedFeatures, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciReadLocalVerInfoCmd
 *
 *  \brief  HCI read local version info command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciReadLocalVerInfoCmd(void)
{
  /* unused */
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeReadLocalVersionInformation, gpBle_ReadLocalVersionInformation, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciReadRemoteVerInfoCmd
 *
 *  \brief  HCI read remote version info command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciReadRemoteVerInfoCmd(uint16_t handle)
{
    gpHci_CommandParameters_t cmd;

    cmd.ReadRemoteVersionInfo.connectionHandle = handle;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeReadRemoteVersionInfo, gpBle_ReadRemoteVersionInfo, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciReadRssiCmd
 *
 *  \brief  HCI read RSSI command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciReadRssiCmd(uint16_t handle)
{
    gpHci_CommandParameters_t cmd;

    cmd.ReadRSSI.connectionHandle = handle;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeReadRSSI, gpBle_ReadRSSI, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciReadTxPwrLvlCmd
 *
 *  \brief  HCI read Tx power level command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciReadTxPwrLvlCmd(uint16_t handle, uint8_t type)
{
    gpHci_CommandParameters_t cmd;

    cmd.ReadTransmitPowerLevel.connectionHandle = handle;
    cmd.ReadTransmitPowerLevel.type = type;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeReadTransmitPowerLevel, gpBle_ReadTransmitPowerLevel , &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciResetCmd
 *
 *  \brief  HCI reset command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciResetCmd(void)
{
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeReset, gpBle_Reset, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciSetEventMaskCmd
 *
 *  \brief  HCI set event mask command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciSetEventMaskCmd(uint8_t *pEventMask)
{
    gpHci_CommandParameters_t cmd;

    MEMCPY(&cmd.SetEventMask, pEventMask, sizeof(cmd.SetEventMask));

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeSetEventMask, gpBle_SetEventMask, &cmd);
}

/*************************************************************************************************/
/*!
 *  \fn     HciVendorSpecificCmd
 *
 *  \brief  HCI vendor specific command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciVendorSpecificCmd(uint16_t opcode, uint8_t len, uint8_t *pData)
{
  /* not used */

}
/*************************************************************************************************/
/*!
*  \fn     HciLeRemoteConnParamReqReply
*
*  \brief  HCI Remote Connection Parameter Request Reply.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeRemoteConnParamReqReply(uint16_t handle, uint16_t intervalMin, uint16_t intervalMax, uint16_t latency,
                                  uint16_t timeout, uint16_t minCeLen, uint16_t maxCeLen)
{
    gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
    cmd.LeRemoteConnectionParamRequestReply.connectionHandle = handle;
    cmd.LeRemoteConnectionParamRequestReply.connIntervalMin = intervalMin;
    cmd.LeRemoteConnectionParamRequestReply.connIntervalMax = intervalMax;
    cmd.LeRemoteConnectionParamRequestReply.connLatency = latency;
    cmd.LeRemoteConnectionParamRequestReply.supervisionTimeout = timeout;
    cmd.LeRemoteConnectionParamRequestReply.minCELength = minCeLen;
    cmd.LeRemoteConnectionParamRequestReply.maxCELength = maxCeLen;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeRemoteConnectionParamRequestReply, gpBle_LeRemoteConnectionParamRequestReply, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeRemoteConnectionParamRequestReply, gpBle_UnknownOpCode, &cmd);
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
}

/*************************************************************************************************/
/*!
*  \fn     HciLeSetDataLen
*
*  \brief  HCI LE Set Data Length.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeSetDataLen(uint16_t handle, uint16_t txOctets, uint16_t txTime)
{
    gpHci_CommandParameters_t cmd;
#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
    cmd.LeSetDataLength.connectionHandle = handle;
    cmd.LeSetDataLength.txOctets = txOctets;
    cmd.LeSetDataLength.txTime = txTime;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetDataLength, gpBle_LeSetDataLength, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetDataLength, gpBle_UnknownOpCode, &cmd);
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
}

/*************************************************************************************************/
/*!
*  \fn     HciWriteAuthPayloadTimeout
*
*  \brief  HCI write authenticated payload timeout command.
*
*  \param  handle    Connection handle.
*  \param  timeout   Timeout value.
*
*  \return None.
*/
/*************************************************************************************************/
void HciWriteAuthPayloadTimeout(uint16_t handle, uint16_t timeout)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
  cmd.WriteAuthenticatedPayloadTO.connectionHandle = handle;
  cmd.WriteAuthenticatedPayloadTO.authenticatedPayloadTO = timeout;

  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeWriteAuthenticatedPayloadTO, gpBle_WriteAuthenticatedPayloadTO,&cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeWriteAuthenticatedPayloadTO, gpBle_UnknownOpCode, &cmd);
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
}

/*************************************************************************************************/
/*!
*  \fn     HciLeRemoteConnParamReqNegReply
*
*  \brief  HCI Remote Connection Parameter Request Negative Reply.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeRemoteConnParamReqNegReply(uint16_t handle, uint8_t reason)
{
    gpHci_CommandParameters_t cmd;
#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
    cmd.LeRemoteConnectionParamRequestNegReply.connectionHandle = handle;
    cmd.LeRemoteConnectionParamRequestNegReply.reason = reason;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeRemoteConnectionParamRequestNegReply, gpBle_LeRemoteConnectionParamRequestNegReply, &cmd);
#else
    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeRemoteConnectionParamRequestNegReply, gpBle_UnknownOpCode, &cmd);
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
}

/*************************************************************************************************/
/*!
 *  \brief  HCI add device to resolving list command.
 *
 *  \param  peerAddrType        Peer identity address type.
 *  \param  pPeerIdentityAddr   Peer identity address.
 *  \param  pPeerIrk            Peer IRK.
 *  \param  pLocalIrk           Local IRK.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeAddDeviceToResolvingListCmd(uint8_t peerAddrType, const uint8_t *pPeerIdentityAddr,
                                      const uint8_t *pPeerIrk, const uint8_t *pLocalIrk)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  cmd.LeAddDeviceToResolvingList.peerIdentityAddressType = peerAddrType;
  MEMCPY(&cmd.LeAddDeviceToResolvingList.peerIdentityAddress, pPeerIdentityAddr,
         sizeof(cmd.LeAddDeviceToResolvingList.peerIdentityAddress));
  MEMCPY(&cmd.LeAddDeviceToResolvingList.peerIRK, pPeerIrk,
         sizeof(cmd.LeAddDeviceToResolvingList.peerIRK));
  MEMCPY(&cmd.LeAddDeviceToResolvingList.localIRK, pLocalIrk,
        sizeof(cmd.LeAddDeviceToResolvingList.localIRK));

  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeAddDeviceToResolvingList,
        gpBle_LeAddDeviceToResolvingList, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeAddDeviceToResolvingList,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
*  \fn     HciLeRemoveDeviceFromResolvingList
*
*  \brief  HCI remove device from resolving list command.
*
*  \param  peerAddrType        Peer identity address type.
*  \param  pPeerIdentityAddr   Peer identity address.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeRemoveDeviceFromResolvingList(uint8_t peerAddrType, const uint8_t *pPeerIdentityAddr)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  cmd.LeRemoveDeviceFromResolvingList.peerIdentityAddressType = peerAddrType;
  MEMCPY(&cmd.LeRemoveDeviceFromResolvingList.peerIdentityAddress, pPeerIdentityAddr,
         sizeof(cmd.LeRemoveDeviceFromResolvingList.peerIdentityAddress));

  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeRemoveDeviceFromResolvingList,
        gpBle_LeRemoveDeviceFromResolvingList, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeRemoveDeviceFromResolvingList,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
*  \fn     HciLeClearResolvingList
*
*  \brief  HCI clear resolving list command.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeClearResolvingList(void)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeClearResolvingList,
        gpBle_LeClearResolvingList, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeClearResolvingList,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
*  \fn     HciLeReadResolvingListSize
*
*  \brief  HCI read resolving list command.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeReadResolvingListSize(void)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadResolvingListSize,
        gpBle_LeReadResolvingListSize, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadResolvingListSize,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
*  \fn     HciLeReadPeerResolvableAddr
*
*  \brief  HCI read peer resolvable address command.
*
*  \param  addrType        Peer identity address type.
*  \param  pIdentityAddr   Peer identity address.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeReadPeerResolvableAddr(uint8_t addrType, const uint8_t *pIdentityAddr)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  cmd.VsdGeneratePeerResolvableAddress.peerIdentityAddressType = addrType;
  MEMCPY(&cmd.VsdGeneratePeerResolvableAddress.peerIdentityAddress, pIdentityAddr,
         sizeof(cmd.VsdGeneratePeerResolvableAddress.peerIdentityAddress));

  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdGeneratePeerResolvableAddress,
        gpBle_VsdGeneratePeerResolvableAddress, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdGeneratePeerResolvableAddress,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
*  \fn     HciLeReadLocalResolvableAddr
*
*  \brief  HCI read local resolvable address command.
*
*  \param  addrType        Peer identity address type.
*  \param  pIdentityAddr   Peer identity address.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeReadLocalResolvableAddr(uint8_t addrType, const uint8_t *pIdentityAddr)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  cmd.VsdGenerateLocalResolvableAddress.peerIdentityAddressType = addrType;
  MEMCPY(&cmd.VsdGenerateLocalResolvableAddress.peerIdentityAddress, pIdentityAddr,
         sizeof(cmd.VsdGenerateLocalResolvableAddress.peerIdentityAddress));

  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdGenerateLocalResolvableAddress,
        gpBle_VsdGenerateLocalResolvableAddress, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdGenerateLocalResolvableAddress,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
*  \fn     HciLeSetAddrResolutionEnable
*
*  \brief  HCI enable or disable address resolution command.
*
*  \param  enable      Set to TRUE to enable address resolution or FALSE to disable address
*                      resolution.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeSetAddrResolutionEnable(uint8_t enable)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  cmd.LeSetAddressResolutionEnable.enable = enable;

  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetAddressResolutionEnable,
        gpBle_LeSetAddressResolutionEnable, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetAddressResolutionEnable,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
*  \fn     HciLeSetResolvablePrivateAddrTimeout
*
*  \brief  HCI set resolvable private address timeout command.
*
*  \param  rpaTimeout    Timeout measured in seconds.
*
*  \return None.
*/
/*************************************************************************************************/
void HciLeSetResolvablePrivateAddrTimeout(uint16_t rpaTimeout)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  cmd.LeSetResolvablePrivateAddressTimeout.rpa_timeout = rpaTimeout;

  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetResolvablePrivateAddressTimeout,
        gpBle_LeSetResolvablePrivateAddressTimeout, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetResolvablePrivateAddressTimeout,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     HciLeSetPrivacyModeCmd
 *
 *  \brief  HCI LE set privacy mode command.
 *
 *  \param  peerAddrType    Peer identity address type.
 *  \param  pPeerAddr       Peer identity address.
 *  \param  mode            Privacy mode.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeSetPrivacyModeCmd(uint8_t addrType, uint8_t *pAddr, uint8_t mode)
{
  gpHci_CommandParameters_t cmd;

#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
  cmd.LeSetPrivacyMode.peerIdentityAddressType = addrType;
  MEMCPY(&cmd.LeSetPrivacyMode.peerIdentityAddress, pAddr,
         sizeof(cmd.LeSetPrivacyMode.peerIdentityAddress));
  cmd.LeSetPrivacyMode.privacyMode = mode;

  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetPrivacyMode,
        gpBle_LeSetPrivacyMode, &cmd);
#else
  gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeSetPrivacyMode,
        gpBle_UnknownOpCode, &cmd);
#endif
}

/*************************************************************************************************/
/*!
 *  \brief      HCI LE request peer SCA command.
 *
 *  \param      handle    Connection handle.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void HciLeRequestPeerScaCmd(uint16_t handle)
{
    //Not supported
}

/*************************************************************************************************/
/*!
 *  \brief      HCI LE set host feature command.
 *
 *  \param      bitNum    Bit position in the FeatureSet.
 *  \param      bitVal    Enable or disable feature.
 *
 *  \return     None.
 *
 *  \note Set or clear a bit in the feature controlled by the Host in the Link Layer FeatureSet
 *  stored in the Controller.
 */
/*************************************************************************************************/
void HciLeSetHostFeatureCmd(uint8_t bitNum, bool_t bitVal)
{
    //Not supported
}



/*************************************************************************************************/
/*!
*  \fn     HciVsdDisableSlaveLatency
*
*  \brief  HCI Vendor specific command to Disable Slave Latency.
*
*  \param  disabled     SL disabled = true, SL enabled = false.
*
*  \return None.
*/
/*************************************************************************************************/
void HciVsdDisableSlaveLatency(uint16_t handle, bool_t disabled)
{
    gpHci_CommandParameters_t cmd;
    cmd.VsdDisableSlaveLatency.connHandle = handle;
    cmd.VsdDisableSlaveLatency.disabled = disabled;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdDisableSlaveLatency, gpBle_VsdDisableSlaveLatency, &cmd);

}

/*************************************************************************************************/
/*!
*  \fn     HciVsdSetDefaultControlledBandwidthValue
*
*  \brief  HCI Vendor specific command to enable Controlled Bandwidth by default
*
*  \param enable True to enable, False to disable
*
*  \return None.
*/
/*************************************************************************************************/
void HciVsdEnableControlledBandwidthModeByDefault(Bool enable)
{
    gpHci_CommandParameters_t cmd;

    cmd.VsdEnCBByDefault.enable = enable;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdEnCBByDefault, gpBle_VsdEnCBByDefault, &cmd);

}

/*************************************************************************************************/
/*!
*  \fn     HciVsdOverruleRemoteMaxRxOctetsAndTime
*
*  \brief  HCI Vendor specific command to overrule the Max Rx Octets and Max Rx Time of the remote peripheral
*
*  \param maxRxOctetsRemote
*  \param maxRxTimeRemote
*
*  \return None.
*/
/*************************************************************************************************/
void HciVsdOverruleRemoteMaxRxOctetsAndTime(uint16_t handle, uint16_t maxRxOctetsRemote, uint16_t maxRxTimeRemote)
{
    gpHci_CommandParameters_t cmd;

    cmd.VsdOverruleRemoteMaxRxOctetsAndTime.connHandle = handle;
    cmd.VsdOverruleRemoteMaxRxOctetsAndTime.maxRxOctetsRemote = maxRxOctetsRemote;
    cmd.VsdOverruleRemoteMaxRxOctetsAndTime.maxRxTimeRemote = maxRxTimeRemote;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdOverruleRemoteMaxRxOctetsAndTime, gpBle_VsdOverruleRemoteMaxRxOctetsAndTime, &cmd);

}


#if defined(GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED)
/*************************************************************************************************/
/*!
*  \fn     HciVsdSetScanCoexParams
*
*  \brief  HCI Vendor specific command to set scan coex settings.
*
*  \param scan_rx_wd_prio         Priority of scan rx window coex request
*  \param scan_rx_wd_req_en       Coex request enabled for scan rx window
*  \param scan_rx_wd_grant_aware  Scan rx window is grant aware
*  \param scan_rx_prio            Priority of scan rx packet coex request
*  \param scan_rx_req_en          Coex request enabled for scan rx packet
*  \param scan_rx_grant_aware     Scan rx packet is grant aware
*  \param scan_tx_prio            Priority of scan tx coex request
*  \param scan_tx_req_en          Coex request enabled for scan tx
*  \param scan_tx_grant_aware     Scan tx is grant aware
*
*  \return None.
*/
/*************************************************************************************************/
void HciVsdSetScanCoexParams(
    UInt8                          scan_rx_wd_prio,
    Bool                           scan_rx_wd_req_en,
    Bool                           scan_rx_wd_grant_aware,
    UInt8                          scan_rx_prio,
    Bool                           scan_rx_req_en,
    Bool                           scan_rx_grant_aware,
    UInt8                          scan_tx_prio,
    Bool                           scan_tx_req_en,
    Bool                           scan_tx_grant_aware)
{
    gpHci_CommandParameters_t cmd;

    cmd.VsdSetScanCoexParams = (gpHci_VsdSetScanCoexParamsCommand_t) {
        .scan_rx_wd = (gpHci_CoexParams_t) {
            .prio   = scan_rx_wd_prio,
            .req    = scan_rx_wd_req_en,
            .grant = scan_rx_wd_grant_aware
        },
        .scan_rx = (gpHci_CoexParams_t) {
            .prio   = scan_rx_prio,
            .req    = scan_rx_req_en,
            .grant = scan_rx_grant_aware
        },
        .scan_tx = (gpHci_CoexParams_t) {
            .prio   = scan_tx_prio,
            .req    = scan_tx_req_en,
            .grant = scan_tx_grant_aware
        }
    };

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeVsdSetScanCoexParams, gpBle_VsdSetScanCoexParams, &cmd);
}
#endif // GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED


/*************************************************************************************************/
/*!
 *  \brief  HCI LE Read Default Data Length.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadDefDataLen(void)
{
  /* not used */
}

/*************************************************************************************************/
/*!
 *  \brief  HCI LE Write Default Data Length.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeWriteDefDataLen(uint16_t suggestedMaxTxOctets, uint16_t suggestedMaxTxTime)
{
  /* unused */
}

/*************************************************************************************************/
/*!
 *  \brief  HCI LE Read Local P-256 Public Key.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadLocalP256PubKey(void)
{
  /* unused */
}

/*************************************************************************************************/
/*!
 *  \brief  HCI LE Generate DH Key.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeGenerateDHKey(uint8_t *pPubKeyX, uint8_t *pPubKeyY)
{
  /* unused */
}

/*************************************************************************************************/
/*!
 *  \brief  HCI LE Generate DH Key Version 2.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeGenerateDHKeyV2(uint8_t *pPubKeyX, uint8_t *pPubKeyY, uint8_t keyType)
{
  /* unused */
}

/*************************************************************************************************/
/*!
 *  \brief  HCI LE Read Maximum Data Length.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciLeReadMaxDataLen(void)
{
  /* not used */
}

/*************************************************************************************************/
/*!
 *  \brief  HCI read authenticated payload timeout command.
 *
 *  \param  handle    Connection handle.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciReadAuthPayloadTimeout(uint16_t handle)
{
  /* not used */
}

/*************************************************************************************************/
/*!
 *  \brief  HCI set event page 2 mask command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciSetEventMaskPage2Cmd(uint8_t *pEventMask)
{
  /* unused */
}
