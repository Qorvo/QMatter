/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   gpBleConnectionQueue is used to queue scanning and advertising requests in case there is no
 *   Declarations of the public functions and enumerations of gpBleConnectionQueue.
 *
 *   This software is owned by Qorvo Inc
 *   and protected under applicable copyright laws.
 *   It is delivered under the terms of the license
 *   and is intended and supplied for use solely and
 *   exclusively with products manufactured by
 *   Qorvo Inc.
 *
 *
 *   THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *   CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *   IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *   LIMITED TO, IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *   QORVO INC. SHALL NOT, IN ANY
 *   CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *   INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *   FOR ANY REASON WHATSOEVER.
 *
 *   $Header$
 *   $Change$
 *   $DateTime$
 */


#ifndef _GPBLECONNECTIONQUEUE_H_
#define _GPBLECONNECTIONQUEUE_H_

/// @file "gpBleConnectionQueue.h"
/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/** @enum gpBleConnectionQueue_Result_t */
//@{
/** @brief Requested Connection Queue action successful */
#define gpBleConnectionQueueResult_Success                     0x00
/** @brief Connection Queue overflow, new element is dropped */
#define gpBleConnectionQueueResult_Overflow                    0x01
/** @brief Connection queue is empty */
#define gpBleConnectionQueueResult_Empty                       0x02
/** @brief Connection Queue does not contain the given link id */
#define gpBleConnectionQueueResult_NotFound                    0x03
/** @brief Given link id is invalid */
#define gpBleConnectionQueueResult_Invalid                     0x04
/** @brief Requested action returns an unexpected result. */
#define gpBleConnectionQueueResult_Unknown                     0xFF
/** @typedef gpBleConnectionQueue_Result_t
    @brief Result types for gpBleConnectionQueue
*/
typedef UInt8                             gpBleConnectionQueue_Result_t;
//@}

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_CONN_QUEUE_LINKID_NONE                              0xFF
#define QUEUE_INVALID                                           0xFF
#ifndef ADVQUEUE_ID
#define ADVQUEUE_ID                                             QUEUE_INVALID
#endif
#ifndef SCANQUEUE_ID
#define SCANQUEUE_ID                                            QUEUE_INVALID
#endif
#if (ADVQUEUE_ID ==  QUEUE_INVALID) ^ (SCANQUEUE_ID == QUEUE_INVALID)
#define QUEUE_AMOUNT                                            1
#else
#define QUEUE_AMOUNT                                            2
#endif

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
typedef UInt8                             stmResult_t;
typedef UInt8                             stmEvent_t;
typedef stmResult_t                       (*stmSendEvent_t) (stmEvent_t stmEvent, UInt8 linkId);

typedef struct ConnectionQueue_Config_s {
    UInt8*              queue;
    UInt8               queue_size;
    stmResult_t         stmResult_NotProcessed;
    stmEvent_t          stmEvent;
    stmSendEvent_t      stmSendEvent;

}ConnectionQueue_Config_t;

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
/** @brief Initialize the Connection queue
*   @return result                    Returns always gpResult_success
*/
gpBleConnectionQueue_Result_t gpBleConnectionQueue_Init(void);

/** @brief Add LinkId of Connection request to queue
*
*   @param LinkId                    LinkId to be added to the queue
*   @return result                   Returns gpResult_success if new request could be added. gpResult_Overflow should not be possible.
*/
gpBleConnectionQueue_Result_t gpBleConnectionQueue_Add(UInt8 LinkId, UInt8 queueId);

/** @brief Remove LinkId of Connection request from queue
*
*   @param LinkId                    LinkId to be removed from the queue
*   @return result                   Returns gpResult_success if new request could be added. gpResult_Overflow should not be possible.
*/
gpBleConnectionQueue_Result_t gpBleConnectionQueue_Remove(UInt8 LinkId, UInt8 queueId);

/** @brief Pop an Connection request from Connection queue
*/
void gpBleConnectionQueue_Pop(UInt8 queueId);


//Indications

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GPBLECONNECTIONQUEUE_H_

