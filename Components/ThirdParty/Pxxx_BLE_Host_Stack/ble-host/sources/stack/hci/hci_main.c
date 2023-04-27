/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  HCI main module.
 *
 *  Copyright (c) 2009-2018 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019-2020 Packetcraft, Inc.
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
 *
 *  This is the main module of the HCI subsystem. It contains the API functions for initialization
 *  and registration.
 */

/*
 * Copyright (c) 2021, Qorvo Inc
 *
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
 */
/*************************************************************************************************/

#include "wsf_types.h"
#include "wsf_msg.h"
#include "hci_api.h"
#include "hci_main.h"

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/* Control block */
hciCb_t hciCb;

/*************************************************************************************************/
/*!
 *  \brief  Register a callback for command complete events to Application
 *          This API expected to use only in cases where stack support for the event is missing.
 *
 *  \param  directEvtCback  Callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciUnhandledCmdComplEvtRegister(hciUnhandledCmdComplEvtCback_t unhandledCmdComplEvtCback)
{
  hciCb.unhandledCmdComplEvtCback = unhandledCmdComplEvtCback;
}

/*************************************************************************************************/
/*!
 *  \brief  Register a callback for HCI events.
 *
 *  \param  evtCback  Callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciEvtRegister(hciEvtCback_t evtCback)
{
  hciCb.evtCback = evtCback;
}

/*************************************************************************************************/
/*!
 *  \brief  Register a callback for certain HCI security events.
 *
 *  \param  secCback  Callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciSecRegister(hciSecCback_t secCback)
{
  hciCb.secCback = secCback;
}

/*************************************************************************************************/
/*!
 *  \brief  Register callbacks for the HCI data path.
 *
 *  \param  aclCback  ACL data callback function.
 *  \param  flowCback Flow control callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciAclRegister(hciAclCback_t aclCback, hciFlowCback_t flowCback)
{
  hciCb.aclCback = aclCback;
  hciCb.flowCback = flowCback;
}

/*************************************************************************************************/
/*!
 *  \brief  Register callbacks for the HCI ISO data path.
 *
 *  \param  isoCback  ISO data callback function.
 *  \param  flowCback Flow control callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciIsoRegister(hciIsoCback_t isoCback, hciFlowCback_t flowCback)
{
  hciCb.isoCback = isoCback;
  hciCb.isoFlowCback = flowCback;
}

/*************************************************************************************************/
/*!
 *  \brief  HCI handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID for HCI.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciHandlerInit(wsfHandlerId_t handlerId)
{
  /* store handler ID */
  hciCb.handlerId = handlerId;

  /* Set ISO call back to free payloads in the case that uninitialized ISO gets an unexpected ISO packet. */
  hciCb.isoCback = (hciIsoCback_t) WsfMsgFree;

  /* init rx queue */
  WSF_QUEUE_INIT(&hciCb.rxQueue);

  /* perform other hci initialization */
  HciCoreInit();
}

/*************************************************************************************************/
/*!
 *  \brief  WSF event handler for HCI.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
  HciCoreHandler(event, pMsg);
}
