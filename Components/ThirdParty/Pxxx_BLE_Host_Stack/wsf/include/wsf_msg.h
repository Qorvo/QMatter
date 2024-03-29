/*************************************************************************************************/
/*!
 *  \file   wsf_msg.h
 *
 *  \brief  Message passing service.
 *
 *  Copyright (c) 2009-2018 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019-2021 Packetcraft, Inc.
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
#ifndef WSF_MSG_H
#define WSF_MSG_H

#include "wsf_queue.h"
#include "wsf_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup WSF_MSG_API
 *  \{ */


/**************************************************************************************************
  Data Types
**************************************************************************************************/

extern const uint8_t WSF_MSG_HDR_SIZE;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Verify whether a data buffer with required length is available to send a
 *          message buffer with WsfMsgSend().
 *
 *  \param  len   Message length in bytes.
 *  \param  tailroom  Tailroom length in bytes.
 *
 *  \return True if buffer is available, false if buffer allocation is not possible.
 */
/*************************************************************************************************/
bool_t CheckWsfMsgDataAlloc(uint16_t len, uint8_t tailroom);

/*************************************************************************************************/
/*!
 *  \brief  Allocate a data message buffer to be sent with WsfMsgSend().
 *
 *  \param  len       Message length in bytes.
 *  \param  tailroom  Tailroom length in bytes.
 *
 *  \return Pointer to data message buffer or NULL if allocation failed.
 */
/*************************************************************************************************/
void *WsfMsgDataAlloc(uint16_t len, uint8_t tailroom);

/*************************************************************************************************/
/*!
 *  \brief  Verify whether a buffer with required length is available to send a
 *          message buffer with WsfMsgSend().
 *
 *  \param  len   Message length in bytes.
 *
 *  \return True if buffer is available, false if buffer allocation is not possible.
 */
/*************************************************************************************************/
bool_t CheckWsfMsgAlloc(uint16_t len);

/*************************************************************************************************/
/*!
 *  \brief  Allocate a message buffer to be sent with WsfMsgSend().
 *
 *  \param  len   Message length in bytes.
 *
 *  \return Pointer to message buffer or NULL if allocation failed.
 */
/*************************************************************************************************/
void *WsfMsgAlloc(uint16_t len);

/*************************************************************************************************/
/*!
 *  \brief  Free a message buffer allocated with WsfMsgAlloc().
 *
 *  \param  pMsg  Pointer to message buffer.
 */
/*************************************************************************************************/
void WsfMsgFree(void *pMsg);

/*************************************************************************************************/
/*!
 *  \brief  Send a message to an event handler.
 *
 *  \param  handlerId   Event handler ID.
 *  \param  pMsg        Pointer to message buffer.
 */
/*************************************************************************************************/
void WsfMsgSend(wsfHandlerId_t handlerId, void *pMsg);

/*************************************************************************************************/
/*!
 *  \brief  Enqueue a message.
 *
 *  \param  pQueue     Pointer to queue.
 *  \param  handlerId  Set message handler ID to this value.
 *  \param  pMsg       Pointer to message buffer.
 */
/*************************************************************************************************/
void WsfMsgEnq(wsfQueue_t *pQueue, wsfHandlerId_t handlerId, void *pMsg);

/*************************************************************************************************/
/*!
 *  \brief  Dequeue a message.
 *
 *  \param  pQueue      Pointer to queue.
 *  \param  pHandlerId  Handler ID of returned message; this is a return parameter.
 *
 *  \return Pointer to message that has been dequeued or NULL if queue is empty.
 */
/*************************************************************************************************/
void *WsfMsgDeq(wsfQueue_t *pQueue, wsfHandlerId_t *pHandlerId);

/*************************************************************************************************/
/*!
 *  \brief  Get the next message without removing it from the queue.
 *
 *  \param  pQueue      Pointer to queue.
 *  \param  pHandlerId  Handler ID of returned message; this is a return parameter.
 *
 *  \return Pointer to the next message on the queue or NULL if queue is empty.
 */
/*************************************************************************************************/
void *WsfMsgPeek(wsfQueue_t *pQueue, wsfHandlerId_t *pHandlerId);

/*************************************************************************************************/
/*!
 *  \brief  Get the Nth message without removing it from the queue.
 *
 *  \param  pQueue      Pointer to queue.
 *  \param  n           Nth item from the top (0 = top element).
 *  \param  pHandlerId  Handler ID of returned message; this is a return parameter.
 *
 *  \return Pointer to the next message on the queue or NULL if queue is empty.
 */
/*************************************************************************************************/
void *WsfMsgNPeek(wsfQueue_t *pQueue, uint8_t n, wsfHandlerId_t *pHandlerId);

/*! \} */    /* WSF_MSG_API */

#ifdef __cplusplus
};
#endif

#endif /* WSF_MSG_H */
