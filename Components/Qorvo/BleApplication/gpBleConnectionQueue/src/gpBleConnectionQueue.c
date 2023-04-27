/*
 *   Copyright (c) 2019, Qorvo Inc
 *
 *   gpBleConnectionQueue is used to queue scanning and advertising requests in case there is no
 *   Implementation of gpBleConnectionQueue
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

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLECONNECTIONQUEUE

/* <CodeGenerator Placeholder> General */

#ifdef GP_DIVERSITY_DEVELOPMENT
#define GP_LOCAL_LOG
#endif //GP_DIVERSITY_DEVELOPMENT

/* </CodeGenerator Placeholder> General */


#include "gpBleConnectionQueue.h"

/* <CodeGenerator Placeholder> Includes */
#include "gpSched.h"
#if   defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_LEGACY_ADVERTISING)
#include "gpBlePeripheralConnectionStm.h"
#endif //(GP_DIVERSITY_BLE_COMBO || GP_DIVERSITY_BLE_ADVSCAN || GP_DIVERSITY_BLE_PERIPHERAL || GP_DIVERSITY_BLE_LEGACY_ADVERTISING) || (GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_LEGACY_SCANNING)
#include "gpLog.h"
#include "gpPoolMem.h"
/* </CodeGenerator Placeholder> Includes */


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */

/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> FunctionalMacro */
/* </CodeGenerator Placeholder> FunctionalMacro */

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> TypeDefinitions */

/* </CodeGenerator Placeholder> TypeDefinitions */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */
#if ADVQUEUE_ID != QUEUE_INVALID
static UInt8 BleAdvQueue[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS];
#endif
#if SCANQUEUE_ID != QUEUE_INVALID
static UInt8 BleScanQueue[GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS];
#endif
static const ConnectionQueue_Config_t BleConnectionQueue_Conf[]=
{
#if ADVQUEUE_ID != QUEUE_INVALID
    {
        BleAdvQueue,
        GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS,
        gpBlePeripheralConnectionStmResult_NotProcessed,
        gpBlePeripheralConnectionStm_EventAdvAvail,
        gpBlePeripheralConnectionStm_SendEvent
    },
#endif //ADVQUEUE_ID != QUEUE_INVALID
#if  SCANQUEUE_ID != QUEUE_INVALID
    {
        BleScanQueue,
        GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS,
        gpBleCentralConnectionStmResult_NotProcessed,
        gpBleCentralConnectionStm_EventScanAvail,
        gpBleCentralConnectionStm_SendEvent
    }
#endif //SCANQUEUE_ID != QUEUE_INVALID

/* ADD NEXT QUEUE CONFIG HERE */
};
/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionPrototypes */
static void gpBleConnectionQueue_Kick(UInt8 queueId);
static void gpBleConnectionQueueItf_SchedPop(void* queueIdSched);
static void gpBleConnectionQueueItf_SchedKick(void* queueIdSched);
static UInt8 gpBleConnectionQueueItf_GetIndex(UInt8 linkId, UInt8 queueId);
/* </CodeGenerator Placeholder> StaticFunctionPrototypes */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */

static void gpBleConnectionQueue_Kick(UInt8 queueId)
{
    //If the first element is not BLE_CONN_QUEUE_LINKID_NONE we have at least 1 item in the queue
    if(BleConnectionQueue_Conf[queueId].queue[0] != BLE_CONN_QUEUE_LINKID_NONE)
    {
        stmResult_t result = BleConnectionQueue_Conf[queueId].stmSendEvent(BleConnectionQueue_Conf[queueId].stmEvent,BleConnectionQueue_Conf[queueId].queue[0]);
        if (result == BleConnectionQueue_Conf[queueId].stmResult_NotProcessed)
        {
           //Received a gpBleApp_resultStmNotProcessed, pop this item from the queue
           //This has to be done in case the state machine is no longer in a scanning state
           //meaning it is not scanning anymore. Popping the queue when that happens
           //prevents the queue from blocking on a unprocessed event
           UInt8* queueIdSched = (UInt8*)GP_POOLMEM_MALLOC(sizeof(UInt8));
           *queueIdSched = queueId;
           gpSched_ScheduleEventArg(0, gpBleConnectionQueueItf_SchedPop, (void*) queueIdSched);
        }
    }
}

static UInt8 gpBleConnectionQueueItf_GetIndex(UInt8 LinkId, UInt8 queueId)
{
    UIntLoop i;

    for(i = 0; i < BleConnectionQueue_Conf[queueId].queue_size; i++)
    {
        if (BleConnectionQueue_Conf[queueId].queue[i] == LinkId)
        {
            return i;
        }
    }

    return BleConnectionQueue_Conf[queueId].queue_size;
}

static void gpBleConnectionQueueItf_Shift(UInt8 idx, UInt8 queueId)
{
    UIntLoop i;

    /* shift forward the remainder of the queue */
    for (i = idx; i < (BleConnectionQueue_Conf[queueId].queue_size - 1); i++)
    {
        BleConnectionQueue_Conf[queueId].queue[i] = BleConnectionQueue_Conf[queueId].queue[i+1];
    }

    /* set the last element to BLE_CONN_QUEUE_LINKID_NONE */
    BleConnectionQueue_Conf[queueId].queue[BleConnectionQueue_Conf[queueId].queue_size - 1] = BLE_CONN_QUEUE_LINKID_NONE;
}

static void gpBleConnectionQueueItf_SchedPop(void* queueIdSched)
{
    UInt8 queueIdLocal = *(UInt8*) queueIdSched;
    gpPoolMem_Free(queueIdSched);
    gpBleConnectionQueue_Pop(queueIdLocal);
}

static void gpBleConnectionQueueItf_SchedKick(void* queueIdSched)
{
    UInt8 queueIdLocal = *(UInt8*) queueIdSched;
    gpPoolMem_Free(queueIdSched);
    gpBleConnectionQueue_Kick(queueIdLocal);
}
/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

gpBleConnectionQueue_Result_t gpBleConnectionQueue_Init(void)
{
/* <CodeGenerator Placeholder> Implementation_gpBleConnectionQueue_Init */
    gpBleConnectionQueue_Result_t result = gpBleConnectionQueueResult_Success;
    UIntLoop queueId = 0;
    UInt8 len = sizeof(BleConnectionQueue_Conf)/sizeof(ConnectionQueue_Config_t);
    //Set all elements to BLE_CONN_QUEUE_LINKID_NONE
    for (queueId = 0; queueId < len; queueId++)
    {
        MEMSET(BleConnectionQueue_Conf[queueId].queue, BLE_CONN_QUEUE_LINKID_NONE, BleConnectionQueue_Conf[queueId].queue_size);
    }
    return result;
/* </CodeGenerator Placeholder> Implementation_gpBleConnectionQueue_Init */
}
gpBleConnectionQueue_Result_t gpBleConnectionQueue_Add(UInt8 LinkId, UInt8 queueId)
{
/* <CodeGenerator Placeholder> Implementation_gpBleConnectionQueue_Add */
    gpBleConnectionQueue_Result_t result = gpBleConnectionQueueResult_Success;
    UIntLoop i;

    if (LinkId >= BleConnectionQueue_Conf[queueId].queue_size)
    {
        GP_LOG_PRINTF("WARNING: invalid linkId %d",0, LinkId);
        result = gpBleConnectionQueueResult_Invalid;
        return result;
    }

    /* look for an existing entry */
    i = gpBleConnectionQueueItf_GetIndex(LinkId, queueId);
    if (i < BleConnectionQueue_Conf[queueId].queue_size)
    {
        GP_LOG_PRINTF("INFO: already queued, linkId %d",0, LinkId);
        result = gpBleConnectionQueueResult_Success;
        return result;
    }

    /* look for an empty entry */
    i = gpBleConnectionQueueItf_GetIndex(BLE_CONN_QUEUE_LINKID_NONE, queueId);
    if (i == BleConnectionQueue_Conf[queueId].queue_size)
    {
        GP_LOG_PRINTF("WARNING: queue overflow",0);
        result = gpBleConnectionQueueResult_Overflow;
        return result;
    }

    /* have an empty entry, use it */
    BleConnectionQueue_Conf[queueId].queue[i] = LinkId;

    /* if first element, kick the queue */
    if (i == 0)
    {
        UInt8* queueIdSched = (UInt8*)GP_POOLMEM_MALLOC(sizeof(UInt8));
        *queueIdSched = queueId;
        gpSched_ScheduleEventArg(0, gpBleConnectionQueueItf_SchedKick, (void*) queueIdSched);
    }
    return result;
/* </CodeGenerator Placeholder> Implementation_gpBleConnectionQueue_Add */
}
gpBleConnectionQueue_Result_t gpBleConnectionQueue_Remove(UInt8 LinkId, UInt8 queueId)
{
/* <CodeGenerator Placeholder> Implementation_gpBleConnectionQueue_Remove */
    gpBleConnectionQueue_Result_t result;
    UIntLoop i;

    if (LinkId >= BleConnectionQueue_Conf[queueId].queue_size)
    {
        GP_LOG_PRINTF("WARNING: invalid linkId %d",0, LinkId);
        result = gpBleConnectionQueueResult_Invalid;
        return result;
    }

    /* look for an existing entry */
    i = gpBleConnectionQueueItf_GetIndex(LinkId, queueId);
    if (i == BleConnectionQueue_Conf[queueId].queue_size)
    {
        GP_LOG_PRINTF("WARNING: not found in the queue, linkId %d",0, LinkId);
        result = gpBleConnectionQueueResult_NotFound;
        return result;
    }

    gpBleConnectionQueueItf_Shift(i, queueId);
    result = gpBleConnectionQueueResult_Success;
    return result;
/* </CodeGenerator Placeholder> Implementation_gpBleConnectionQueue_Remove */
}
void gpBleConnectionQueue_Pop(UInt8 queueId)
{
/* <CodeGenerator Placeholder> Implementation_gpBleConnectionQueue_Pop */
    /* if the queue is not empty, shift it */
    if (BleConnectionQueue_Conf[queueId].queue[0] != BLE_CONN_QUEUE_LINKID_NONE)
    {
        gpBleConnectionQueueItf_Shift(0, queueId);
    }

    /* if the queue is not empty, kick it */
    if (BleConnectionQueue_Conf[queueId].queue[0] != BLE_CONN_QUEUE_LINKID_NONE)
    {
        UInt8* queueIdSched = (UInt8*)GP_POOLMEM_MALLOC(sizeof(UInt8));
        *queueIdSched = queueId;
        gpSched_ScheduleEventArg(0, gpBleConnectionQueueItf_SchedKick, (void*) queueIdSched);
    }
/* </CodeGenerator Placeholder> Implementation_gpBleConnectionQueue_Pop */
}
