/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  HCI transport interface.
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
#ifndef HCI_TR_H
#define HCI_TR_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Send a complete HCI ACL packet to the transport.
 *
 *  \param  pContext    Connection context.
 *  \param  pAclData    WSF msg buffer containing an ACL packet.
 *  \param  hciFraglen  Length of the HCI fragment.
 *  \param  hciFragPb   HCI Packet boundary flag.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendAclData(void *pContext, uint8_t *pAclData, uint16_t hciFragLen, uint16_t hciFragPb);

/*************************************************************************************************/
/*!
 *  \brief  Send a complete HCI ISO ACL packet to the transport.
 *
 *  \param  pTxCb       Transmit context.
 *  \param  isoPkt      ISO Packet.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendIsoData(void *pContext, uint8_t *pData);

/*************************************************************************************************/
/*!
 *  \brief  Send a complete HCI command to the transport.
 *
 *  \param  pCmdData    WSF msg buffer containing an HCI command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendCmd(uint8_t *pCmdData);

/*************************************************************************************************/
/*!
 *  \brief  Initialize HCI transport resources.
 *
 *  \param  port        COM port.
 *  \param  baudRate    Baud rate.
 *  \param  flowControl TRUE if flow control is enabled
 *
 *  \return TRUE if initialization succeeds, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t hciTrInit(uint8_t port, uint32_t baudRate, bool_t flowControl);

/*************************************************************************************************/
/*!
 *  \brief  Close HCI transport resources.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrShutdown(void);

#ifdef __cplusplus
};
#endif

#endif /* HCI_TR_H */
