/*!
 *  \file   hci_core_ps.c
 *
 *  \brief  HCI core platform-specific module single-chip.
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
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "bda.h"
#include "bstream.h"
#include "hci_core.h"
#include "hci_tr.h"
#include "hci_cmd.h"
#include "hci_evt.h"
#include "hci_api.h"
#include "hci_main.h"

#include "gpLog.h"
#define GP_COMPONENT_ID GP_COMPONENT_ID_QORVOBLEHOST

#include "gpBle.h"
#include "gpBleAddressResolver.h"

#if defined(GP_DIVERSITY_BLE_SLAVE) || defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_ADVERTISER)
#include "gpBleAdvertiser.h"
#endif // GP_DIVERSITY_BLE_SLAVE || GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_ADVERTISER


/**************************************************************************************************
  Macros
**************************************************************************************************/

static void BleHost_GetHostBufferSize(void)
{
    gpHci_CommandParameters_t cmd;

    gpBle_ExecuteCommand(gpBle_EventServer_Host, gpHci_OpCodeLeReadBufferSize, gpBle_LeReadBufferSize, &cmd);
}

uint8_t LlGetResolvingListSize(void)
{
    return 0;
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreInit
 *
 *  \brief  HCI core initialization.
 *
 *  \return None.
 */
/*************************************************************************************************/

void hciCoreInit(void)
{
    /* Before reset, first get buffer sizes from BLE Controller */
    BleHost_GetHostBufferSize();

    /* if LL Privacy is supported by Controller and included */
    if ((HciGetLeSupFeat() & HCI_LE_SUP_FEAT_PRIVACY) &&
        (hciLeSupFeatCfg & HCI_LE_SUP_FEAT_PRIVACY))
    {
        hciCoreCb.resListSize = LlGetResolvingListSize();
    }
    else
    {
        hciCoreCb.resListSize = 0;
    }
}

/*************************************************************************************************/
/*!
 *  \fn     hciCoreNumCmplPkts
 *
 *  \brief  Handle an HCI Number of Completed Packets event.
 *
 *  \param  handle    Connection handle.
 *  \param  numBufs   Number of buffers completed.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCoreNumCmplPkts(uint16_t handle, uint8_t numBufs)
{
  hciCoreConn_t   *pConn;

  if ((pConn = hciCoreConnByHandle(handle)) != NULL)
  {
    /* decrement outstanding buffer count to controller */
    pConn->outBufs -= (uint8_t) numBufs;

    /* decrement queued buffer count for this connection */
    pConn->queuedBufs -= (uint8_t) numBufs;

    /* call flow control callback */
    if (pConn->flowDisabled && pConn->queuedBufs <=  hciCoreCb.aclQueueLo)
    {
      pConn->flowDisabled = FALSE;
      (*hciCb.flowCback)(handle, FALSE);
    }

    /* service TX data path */
    hciCoreTxReady(numBufs);
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciCoreHandler
 *
 *  \brief  WSF event handler for core HCI.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciCoreHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
  uint8_t         *pBuf;
  wsfHandlerId_t  handlerId;

  /* Handle events */
  if (event & HCI_EVT_RX)
  {
    /* Process rx queue */
    while ((pBuf = WsfMsgDeq(&hciCb.rxQueue, &handlerId)) != NULL)
    {
      /* Handle ACL data */
      if(handlerId == HCI_ACL_TYPE)
      {
        /* Reassemble */
        if ((pBuf = hciCoreAclReassembly(pBuf)) != NULL)
        {
          /* Call ACL callback; client will free buffer */
          hciCb.aclCback(pBuf);
        }
      }
      else
      {
        GP_LOG_SYSTEM_PRINTF("rx queue ?",0);
        /* Free buffer */
        WsfMsgFree(pBuf);
      }
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetBdAddr
 *
 *  \brief  Return a pointer to the BD address of this device.
 *
 *  \return Pointer to the BD address.
 */
/*************************************************************************************************/
uint8_t *HciGetBdAddr(void)
{
  return hciCoreCb.bdAddr;
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetWhiteListSize
 *
 *  \brief  Return the white list size.
 *
 *  \return White list size.
 */
/*************************************************************************************************/
uint8_t HciGetWhiteListSize(void)
{
    gpHci_CommandParameters_t cmd;
    gpBle_EventBuffer_t evt;

    gpBle_LeReadWhiteListSize(&cmd, &evt);

    return evt.payload.commandCompleteParams.returnParams.whiteListSize;
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetAdvTxPwr
 *
 *  \brief  Return the advertising transmit power.
 *
 *  \return Advertising transmit power.
 */
/*************************************************************************************************/
int8_t HciGetAdvTxPwr(void)
{
#if defined(GP_DIVERSITY_BLE_ADVERTISER)
    gpHci_CommandParameters_t cmd;
    gpBle_EventBuffer_t evt;

    gpBle_LeReadAdvertisingChannelTxPower(&cmd, &evt);

    return evt.payload.commandCompleteParams.returnParams.advChannelTxPower;
#else
    return -1;
#endif
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetBufSize
 *
 *  \brief  Return the ACL buffer size supported by the controller.
 *
 *  \return ACL buffer size.
 */
/*************************************************************************************************/
uint16_t HciGetBufSize(void)
{
  return hciCoreCb.bufSize;
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetNumBufs
 *
 *  \brief  Return the number of ACL buffers supported by the controller.
 *
 *  \return Number of ACL buffers.
 */
/*************************************************************************************************/
uint8_t HciGetNumBufs(void)
{
  return hciCoreCb.numBufs;
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetSupStates
 *
 *  \brief  Return the states supported by the controller.
 *
 *  \return Pointer to the supported states array.
 */
/*************************************************************************************************/
uint8_t *HciGetSupStates(void)
{
    static uint8_t supStates[8];

    return supStates;
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetLeSupFeat
 *
 *  \brief  Return the LE supported features supported by the controller.
 *
 *  \return Supported features.
 */
/*************************************************************************************************/
uint64_t HciGetLeSupFeat(void)
{
  return hciCoreCb.leSupFeat;
}
/*************************************************************************************************/
/*!
 *  \brief  Return the LE supported features supported by the controller.
 *
 *  \return Supported features.
 */
/*************************************************************************************************/
uint32_t HciGetLeSupFeat32(void)
{
  /* NOT USED */
  return 0;
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetMaxRxAclLen
 *
 *  \brief  Get the maximum reassembled RX ACL packet length.
 *
 *  \return ACL packet length.
 */
/*************************************************************************************************/
uint16_t HciGetMaxRxAclLen(void)
{
  return hciCoreCb.maxRxAclLen;
}

/*************************************************************************************************/
/*!
 *  \fn     HciGetResolvingListSize
 *
 *  \brief  Return the resolving list size.
 *
 *  \return resolving list size.
 */
/*************************************************************************************************/
uint8_t HciGetResolvingListSize(void)
{
  return hciCoreCb.resListSize;
}

/*************************************************************************************************/
/*!
*  \fn     HciLlPrivacySupported
*
*  \brief  Whether LL Privacy is supported.
*
*  \return TRUE if LL Privacy is supported. FALSE, otherwise.
*/
/*************************************************************************************************/
bool_t HciLlPrivacySupported(void)
{
  return (hciCoreCb.resListSize > 0) ? TRUE : FALSE;
}

/*************************************************************************************************/
/*!
*  \brief  Get the maximum advertisement (or scan response) data length supported by the Controller.
*
*  \return Maximum advertisement data length.
*/
/*************************************************************************************************/
uint16_t HciGetMaxAdvDataLen(void)
{
  return hciCoreCb.maxAdvDataLen;
}

/*************************************************************************************************/
/*!
*  \brief  Get the maximum number of advertising sets supported by the Controller.
*
*  \return Maximum number of advertising sets.
*/
/*************************************************************************************************/
uint8_t HciGetNumSupAdvSets(void)
{
  return hciCoreCb.numSupAdvSets;
}

/*************************************************************************************************/
/*!
*  \brief  Whether LE Advertising Extensions is supported.
*
*  \return TRUE if LE Advertising Extensions is supported. FALSE, otherwise.
*/
/*************************************************************************************************/
bool_t HciLeAdvExtSupported(void)
{
  return (hciCoreCb.numSupAdvSets > 0) ? TRUE : FALSE;
}

/*************************************************************************************************/
/*!
 *  \brief  Return the periodic advertising list size.
 *
 *  \return periodic advertising list size.
 */
/*************************************************************************************************/
uint8_t HciGetPerAdvListSize(void)
{
  return hciCoreCb.perAdvListSize;
}

/*************************************************************************************************/
/*!
 *  \brief  Return a pointer to the local version information.
 *
 *  \return Pointer to the local version information.
 */
/*************************************************************************************************/
hciLocalVerInfo_t *HciGetLocalVerInfo(void)
{
  return &hciCoreCb.locVerInfo;
}
