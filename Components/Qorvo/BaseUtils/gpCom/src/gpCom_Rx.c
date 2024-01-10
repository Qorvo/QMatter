/*
 * Copyright (c) 2014-2016, GreenPeak Technologies
 * Copyright (c) 2017-2019, Qorvo Inc
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
 * Alternatively, this software may be distributed under the terms of the
 * modified BSD License or the 3-clause BSD License as published by the Free
 * Software Foundation @ https://directory.fsf.org/wiki/License:BSD-3-Clause
 *
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG
#define GP_COMPONENT_ID GP_COMPONENT_ID_COM

#include "hal.h"
#include "gpCom.h"
#include "gpCom_defs.h"

#include "gpLog.h"
#include "gpAssert.h"

#ifdef GP_COMP_SCHED
#include "gpSched.h"
#endif //GP_COMP_SCHED

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#ifndef GP_COM_RX_PACKET_BUFFERS
#if !defined(GP_COM_DIVERSITY_MULTIPLE_COM)
#define GP_COM_RX_PACKET_BUFFERS 1
#elif defined(GP_DIVERSITY_LINUXKERNEL)
//Reserve minimum space for possible simultaneous indication and ack
#define GP_COM_RX_PACKET_BUFFERS (2*(GP_COM_IOCTL_KERNEL_NR_OF_CLIENTS))
#else
//Reserve minimum space for possible simultaneous indication and ack
#define GP_COM_RX_PACKET_BUFFERS 2
#endif //GP_DIVERSITY_COM_SHMEM && !defined(GP_COM_DIVERSITY_MULTIPLE_COM)
#endif //GP_COM_RX_PACKET_BUFFERS

#define GP_COM_UNKNOWN_MODULE_ID                   0
#ifndef GP_COM_MAX_NUMBER_OF_MODULE_IDS
#define GP_COM_MAX_NUMBER_OF_MODULE_IDS            2 //16
#endif //GP_COM_MAX_NUMBER_OF_MODULE_IDS

#define BITS_IN_BYTE 8

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct gpCom_ModuleIDCallbackEntry {
    UInt8                  moduleID;
    gpCom_HandleCallback_t handleCallback;
} gpCom_ModuleIDCallbackEntry_t;

#ifdef GP_COM_DIVERSITY_PACKET_FILTERING
typedef struct
{
    UInt8 bufferUsageThresh; // A threshold of RX queue usage in percents
    UInt8 moduleId;
    UInt8 patternsCnt;
    gpCom_FilterPattern_t patterns[GP_COM_FILTER_PATTERN_MAX_CNT];
    Bool enabled;
} gpCom_FilterData_t;
#endif //GP_COM_DIVERSITY_PACKET_FILTERING
/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static gpCom_Packet_t*     gpCom_PacketHandlingQueue[GP_COM_RX_PACKET_BUFFERS];
static gpCom_Packet_t gpCom_PacketBuffer[GP_COM_RX_PACKET_BUFFERS] GP_EXTRAM_SECTION_ATTR;
static UInt8 gpCom_PacketBufferClaimed[(GP_COM_RX_PACKET_BUFFERS-1)/8+1];

static gpCom_ModuleIDCallbackEntry_t gpCom_ModuleIDCallbackTable[GP_COM_MAX_NUMBER_OF_MODULE_IDS];

HAL_CRITICAL_SECTION_DEF(Com_RxMutex)
HAL_CRITICAL_SECTION_DEF(Com_MultiThreadingMutex)
#if defined(GP_COM_DIVERSITY_UNLOCK_TX_AFTER_RX)
extern Bool gpCom_TxLocked;
#endif

#ifdef GP_COM_DIVERSITY_PACKET_FILTERING
static gpCom_FilterData_t filters[GP_COM_FILTER_CNT];
#endif //def GP_COM_DIVERSITY_PACKET_FILTERING

/*****************************************************************************
 *                    External Data Definition
 *****************************************************************************/

/*****************************************************************************
 *                    Internal Function Prototypes
 *****************************************************************************/

//Packet memory handlers
gpCom_Packet_t* Com_GetFreePacket(void);
void            Com_FreePacket(gpCom_Packet_t* pPacket);
//Queue handler
Bool            Com_AddPendingPacket(gpCom_Packet_t* pPacket);

void Com_InitRx(void);
void Com_ConnectionClose(gpCom_CommunicationId_t commId);

void Com_Execute(gpCom_Packet_t * pPacket);

void gpCom_HandleRxNonBlocking(void);
void Com_ExecuteAndFreePacket(gpCom_Packet_t* pPacket, Bool blockingWait, UInt8 cmdId);

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
static void Com_ShiftHandlingQueue(UInt8 startIndex);
static gpCom_Packet_t* Com_GetPendingPacketWithCmdId(UInt8 cmdId);
static gpCom_Packet_t* Com_GetPendingPacket(void);


#ifdef GP_COM_DIVERSITY_PACKET_FILTERING
static Bool isFilterPatternInPacket(gpCom_FilterPattern_t* pPattern, gpCom_Packet_t* pPacket);
static Bool isPatternEqual(const gpCom_FilterPattern_t* pInPattern, const gpCom_FilterPattern_t* pOutPattern, UInt8 patternsCnt);
static UInt8 getFilterIdx(UInt8 threshold);
#endif //def GP_COM_DIVERSITY_PACKET_FILTERING
/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
#ifdef GP_LOCAL_LOG
void Com_DumpRxBuffers(void)
{
    UIntLoop i;
    for (i=0; i < GP_COM_RX_PACKET_BUFFERS; i++)
    {
        gpCom_Packet_t* pPacket = &gpCom_PacketBuffer[i];
        GP_LOG_SYSTEM_PRINTF("mem- %i:%i id:%x l:%i comm:%lu [%x",0,i, BIT_TST(gpCom_PacketBufferClaimed[i/8],i%8),
                                                      pPacket->moduleID,
                                                      pPacket->length,
                                                      (unsigned long)pPacket->commId,
                                                      pPacket->packet[0]);
        gpLog_Flush();
    }
    for (i=0; i < GP_COM_RX_PACKET_BUFFERS; i++)
    {
        if(gpCom_PacketHandlingQueue[i])
        {
            gpCom_Packet_t* pPacket = gpCom_PacketHandlingQueue[i];

            GP_LOG_SYSTEM_PRINTF("que- %i:id:%x l:%i comm:%lu [%x",0,i, pPacket->moduleID,
                                                      pPacket->length,
                                                      (unsigned long)pPacket->commId,
                                                      pPacket->packet[0]);
            gpLog_Flush();
        }
    }
}
#endif

//Memory Claim/Free
gpCom_Packet_t* Com_GetFreePacket(void)
{
    UIntLoop i;
    gpCom_Packet_t *pPacket = NULL;

    if(!HAL_VALID_MUTEX(Com_RxMutex))
    {
        //Rx not initialized yet
        return NULL;
    }

    HAL_ACQUIRE_MUTEX(Com_RxMutex);
    for(i=0; i < GP_COM_RX_PACKET_BUFFERS; i++)
    {
        if(!BIT_TST(gpCom_PacketBufferClaimed[i/8], i%8))
        {
            GP_LOG_PRINTF("G %i",0,i);
            BIT_SET(gpCom_PacketBufferClaimed[i/8], i%8);
            pPacket = &(gpCom_PacketBuffer[i]);
            break;
        }
    }
    HAL_RELEASE_MUTEX(Com_RxMutex);

    if(NULL == pPacket)
    {
        GP_LOG_PRINTF("No Rx buf",0);
#ifdef GP_LOCAL_LOG
        Com_DumpRxBuffers();
#endif //GP_LOCAL_LOG
    }
    return pPacket;
}

void Com_FreePacket(gpCom_Packet_t* pPacket)
{
    UIntLoop i;

    if(!HAL_VALID_MUTEX(Com_RxMutex))
    {
        //Rx not initialized yet
        return;
    }

    HAL_ACQUIRE_MUTEX(Com_RxMutex);
    for(i=0; i < GP_COM_RX_PACKET_BUFFERS; i++)
    {
        if(pPacket == &(gpCom_PacketBuffer[i]))
        {
            GP_LOG_PRINTF("Fr %i",0,i);
            BIT_CLR(gpCom_PacketBufferClaimed[i/8], i%8);
            break;
        }
    }
    HAL_RELEASE_MUTEX(Com_RxMutex);

    if(i == GP_COM_RX_PACKET_BUFFERS)
    {
        GP_LOG_PRINTF("F: packet not found: id:%x l:%i comm:%lu",0,pPacket->moduleID,
                                                                  pPacket->length,
                                                                  (unsigned long)pPacket->commId);
        GP_ASSERT_DEV_EXT(false);
    }
}

//Handling queue - preserve in order execution
static void Com_ShiftHandlingQueue(UInt8 startIndex)
{
    // always called from inside mutex, so no mutex here !
#if GP_COM_RX_PACKET_BUFFERS > 1
    UIntLoop i;
    GP_ASSERT_DEV_INT(startIndex < GP_COM_RX_PACKET_BUFFERS);
    //Shift to front and add NULL at the end
    for(i = startIndex; i < GP_COM_RX_PACKET_BUFFERS-1; i++)
    {
        gpCom_PacketHandlingQueue[i] = gpCom_PacketHandlingQueue[i+1];
    }
#endif //GP_COM_RX_PACKET_BUFFERS > 1
    gpCom_PacketHandlingQueue[GP_COM_RX_PACKET_BUFFERS-1] = NULL;
}

Bool Com_AddPendingPacket(gpCom_Packet_t* pPacket)
{
    UIntLoop i;

    if(!HAL_VALID_MUTEX(Com_RxMutex))
    {
        //Rx not initialized yet
        return false;
    }

    HAL_ACQUIRE_MUTEX(Com_RxMutex);
    for(i=0; i < GP_COM_RX_PACKET_BUFFERS; i++)
    {
        if(gpCom_PacketHandlingQueue[i] == NULL)
        {
            GP_LOG_PRINTF("Add %i",0, i);
            gpCom_PacketHandlingQueue[i] = pPacket;
            break;
        }
    }
    HAL_RELEASE_MUTEX(Com_RxMutex);

    return i != GP_COM_RX_PACKET_BUFFERS;
}

static gpCom_Packet_t* Com_GetPendingPacket(void)
{
    gpCom_Packet_t *pPacket = NULL;

    if(!HAL_VALID_MUTEX(Com_RxMutex))
    {
        //Rx not initialized yet
        return NULL;
    }

    Com_FlushRx();
    HAL_ACQUIRE_MUTEX(Com_RxMutex);
    pPacket = gpCom_PacketHandlingQueue[0];
    if(pPacket != NULL)
    {
        Com_ShiftHandlingQueue(0);
    }
    HAL_RELEASE_MUTEX(Com_RxMutex);

    return pPacket;
}

static gpCom_Packet_t* Com_GetPendingPacketWithCmdId(UInt8 cmdId)
{
    UIntLoop i;
    gpCom_Packet_t *pPacket = NULL;

    if(!HAL_VALID_MUTEX(Com_RxMutex))
    {
        //Rx not initialized yet
        return NULL;
    }

    Com_FlushRx();
    HAL_ACQUIRE_MUTEX(Com_RxMutex);
    for(i=0; i < GP_COM_RX_PACKET_BUFFERS; i++)
    {
        if((gpCom_PacketHandlingQueue[i] != NULL)
        && (gpCom_PacketHandlingQueue[i]->packet[0] == cmdId))
        {
            pPacket = gpCom_PacketHandlingQueue[i];
            Com_ShiftHandlingQueue(i);
            break;
        }
    }
    HAL_RELEASE_MUTEX(Com_RxMutex);

    return pPacket;
}

void Com_InitRx(void)
{
    UIntLoop i;
    for (i=0;i < GP_COM_MAX_NUMBER_OF_MODULE_IDS;i++)
    {
        gpCom_ModuleIDCallbackTable[i].moduleID       = GP_COM_UNKNOWN_MODULE_ID;
        gpCom_ModuleIDCallbackTable[i].handleCallback = NULL;
    }
    for (i=0; i < GP_COM_RX_PACKET_BUFFERS; i++)
    {
        gpCom_PacketHandlingQueue[i] = NULL;
    }
#ifdef GP_COM_DIVERSITY_PACKET_FILTERING
    for(i = 0; i < GP_COM_FILTER_CNT; i++)
    {
        filters[i].enabled = false;
    }
#endif //def GP_COM_DIVERSITY_PACKET_FILTERING
    MEMSET(gpCom_PacketBufferClaimed, 0x0, sizeof(gpCom_PacketBufferClaimed));
    HAL_CREATE_MUTEX(&Com_RxMutex);
    HAL_CREATE_MUTEX(&Com_MultiThreadingMutex);
#if defined(GP_COM_DIVERSITY_UNLOCK_TX_AFTER_RX)
    gpCom_TxLocked = true;
#endif
}

void Com_DeInitRx(void)
{
    if(HAL_VALID_MUTEX(Com_RxMutex))
    {
        HAL_DESTROY_MUTEX(&Com_RxMutex);
    }
    if(HAL_VALID_MUTEX(Com_MultiThreadingMutex))
    {
        HAL_DESTROY_MUTEX(&Com_MultiThreadingMutex);
    }
}

void Com_Execute(gpCom_Packet_t* pPacket)
{
    UIntLoop i;
    //GP_LOG_SYSTEM_PRINTF("Ex:%x %i %lx [%x",0, pPacket->moduleID, pPacket->length, (unsigned long int)pPacket->commId, pPacket->packet[0]);

#if defined(GP_COM_COMM_ID_BRIDGE_1) && defined(GP_COM_COMM_ID_BRIDGE_2)
    gpCom_DataRequest(pPacket->moduleID, pPacket->length, pPacket->packet, (pPacket->commId == GP_COM_COMM_ID_BRIDGE_1) ? \
                                                                                               GP_COM_COMM_ID_BRIDGE_2 : \
                                                                                               GP_COM_COMM_ID_BRIDGE_1);
#endif /* GP_COM_COMM_ID_BRIDGE_1/GP_COM_COMM_ID_BRIDGE_2 */


    for (i=0;i < GP_COM_MAX_NUMBER_OF_MODULE_IDS;i++)
    {
        if (gpCom_ModuleIDCallbackTable[i].moduleID == pPacket->moduleID)
        {
            if (gpCom_ModuleIDCallbackTable[i].handleCallback != NULL)
            {
                gpCom_ModuleIDCallbackTable[i].handleCallback(pPacket->length, pPacket->packet, pPacket->commId);
                break;
            }
        }
        else
        {
            GP_LOG_PRINTF("No %x-%x",2,(UInt16)pPacket->moduleID,(UInt16)gpCom_ModuleIDCallbackTable[i].moduleID);
        }
    }
}

void Com_ConnectionClose(gpCom_CommunicationId_t commId)
{
    UIntLoop i;

    GP_LOG_PRINTF("Close %lx",2, (long unsigned int)commId);
    for (i=0;i < GP_COM_MAX_NUMBER_OF_MODULE_IDS;i++)
    {
        //Notify all modules of connection loss
        if (gpCom_ModuleIDCallbackTable[i].handleCallback != NULL)
        {
            gpCom_ModuleIDCallbackTable[i].handleCallback(0, NULL, commId);
        }
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

Bool gpCom_RegisterModule(UInt8 moduleID, gpCom_HandleCallback_t handleCallback)
{
    UIntLoop i;
    //check for unknown module id
    if (moduleID == GP_COM_UNKNOWN_MODULE_ID)
    {
        GP_ASSERT_DEV_EXT(false);   // proper handling provided (might be omitted)
        return false;
    }

    //search for entry matching module id in binding table
    //search for free entry in binding table
    for (i =0;i<GP_COM_MAX_NUMBER_OF_MODULE_IDS;i++)
    {
        if((gpCom_ModuleIDCallbackTable[i].moduleID == moduleID) ||
           (gpCom_ModuleIDCallbackTable[i].moduleID == GP_COM_UNKNOWN_MODULE_ID))
        {
            gpCom_ModuleIDCallbackTable[i].moduleID       = moduleID;
            gpCom_ModuleIDCallbackTable[i].handleCallback = handleCallback;
            return true;
        }
    }

    GP_LOG_SYSTEM_PRINTF("ERROR: REGISTER GPCOM SERVER %02x",2,(UInt16)moduleID);
    return false;
}

Bool gpCom_DeRegisterModule(UInt8 moduleID)
{
    UIntLoop i;

    if (moduleID == GP_COM_UNKNOWN_MODULE_ID)
    {
        GP_ASSERT_DEV_EXT(false);   // proper handling provided (might be omitted)
        return false;
    }

    //search for entry matching module id in binding table
    for (i =0;i<GP_COM_MAX_NUMBER_OF_MODULE_IDS;i++)
    {
        if(gpCom_ModuleIDCallbackTable[i].moduleID == moduleID)
        {
            gpCom_ModuleIDCallbackTable[i].moduleID       = GP_COM_UNKNOWN_MODULE_ID;
            gpCom_ModuleIDCallbackTable[i].handleCallback = NULL;
            return true;
        }
    }

    return false;
}


Bool gpCom_IsReceivedPacketPending(void)
{
    return (NULL != gpCom_PacketHandlingQueue[0]);
}

void gpCom_HandleRx(void)
{
        gpCom_HandleRxNonBlocking();
}

void gpCom_HandleRxBlocking(Bool blockingWait, UInt8 cmdId)
{
    gpCom_Packet_t* pPacket;

    pPacket = Com_GetPendingPacketWithCmdId(cmdId);
    Com_ExecuteAndFreePacket(pPacket, blockingWait, cmdId);
}

void gpCom_HandleRxNonBlocking(void)
{
    gpCom_Packet_t* pPacket = 0;

    /* The NonBlocking function is called from the main loop of the scheduler */
    /* and therefor HAS TO compete for the multithreading mutex with the function */
    /* gpCom_DataRequestAndWaitForAck() which might be called from other threads */
        pPacket = Com_GetPendingPacket();
    Com_ExecuteAndFreePacket(pPacket, false, 0);
}

void Com_ExecuteAndFreePacket(gpCom_Packet_t* pPacket, Bool blockingWait, UInt8 cmdId)
{
    NOT_USED(blockingWait);
    NOT_USED(cmdId);
    if(NULL != pPacket)
    {
        GP_LOG_PRINTF("Ex m:%x l:%i [%x",0, pPacket->moduleID, pPacket->length, pPacket->packet[0]);

        //Execute packet content
        Com_Execute(pPacket);

        //Free handled packet memory
        Com_FreePacket(pPacket);
    }
}

void Com_cbPacketReceived(gpCom_Packet_t* pPacket)
{
#if defined(GP_COM_DIVERSITY_UNLOCK_TX_AFTER_RX)
    if (gpCom_TxLocked)
    {
        {
            gpCom_TxLocked = false;
            Com_TriggerTx(pPacket->commId);
        }
    }
#endif


#ifdef GP_COM_DIVERSITY_PACKET_FILTERING
    // Check if the packet should be dropped:
    if(gpCom_FilterIsPacketToReject(pPacket, gpCom_RxBufferUsagePercent()))
    {
        Com_FreePacket(pPacket);
        return;
    }
#endif //def GP_COM_DIVERSITY_PACKET_FILTERING

    //GP_LOG_SYSTEM_PRINTF("Add:%x l:%u %lx [%x", 0, state->moduleID, state->length, (unsigned long)state->commId, state->pPacket->packet[0]);
    if(!Com_AddPendingPacket(pPacket))
    {
        Com_FreePacket(pPacket);
        GP_LOG_SYSTEM_PRINTF("No space to handle",0);
        GP_ASSERT_SYSTEM(false);
        return;
    }

#ifdef GP_COMP_SCHED
    gpSched_Trigger();
#endif //GP_COMP_SCHED
}

#ifdef GP_COM_DIVERSITY_PACKET_FILTERING
UInt16 gpCom_ClaimedBuffersCnt(void)
{
    UInt8 i;
    UInt16 bufClaimed = 0;

    if(!HAL_VALID_MUTEX(Com_RxMutex))
    {
        //Rx not initialized yet
        return 0;
    }

    HAL_ACQUIRE_MUTEX(Com_RxMutex);
    for(i=0; i < GP_COM_RX_PACKET_BUFFERS; i++)
    {
        if(BIT_TST(gpCom_PacketBufferClaimed[i / BITS_IN_BYTE], i % BITS_IN_BYTE))
        {
           bufClaimed++;
        }
    }
    HAL_RELEASE_MUTEX(Com_RxMutex);

    return bufClaimed;
}

UInt8 gpCom_RxBufferUsagePercent(void)
{
    return ((gpCom_ClaimedBuffersCnt() * 100) / GP_COM_RX_PACKET_BUFFERS);
}

Bool gpCom_AddFilter(UInt8 moduleId, UInt8 bufferUsageThresh, UInt8 patternsCnt, const gpCom_FilterPattern_t* patterns)
{
    UInt8 i;
    UInt8 idx;
    if(!patterns || (patternsCnt > GP_COM_FILTER_PATTERN_MAX_CNT) || (bufferUsageThresh >= 100) || (filters[GP_COM_FILTER_CNT - 1].enabled))
    {
        return false;
    }
    for(i = 0; i < patternsCnt; i++)
    {
        if(!patterns[i].len || (patterns[i].len > GP_COM_FILTER_PATTERN_MAX_LEN))
        {
            return false;
        }
    }
    idx = getFilterIdx(bufferUsageThresh);
    if (idx >= GP_COM_FILTER_CNT)
    {
        return false;
    }
    filters[idx].enabled = true;
    filters[idx].moduleId = moduleId;
    filters[idx].bufferUsageThresh = bufferUsageThresh;
    filters[idx].patternsCnt = patternsCnt;

    for(i = 0; i < patternsCnt; i++)
    {
        memcpy(filters[idx].patterns[i].data, patterns[i].data, patterns[i].len);

        filters[idx].patterns[i].len = patterns[i].len;
        filters[idx].patterns[i].offset = patterns[i].offset;
    }

    return true;
}

UInt8 gpCom_RemoveFilter(UInt8 moduleId, UInt8 bufferUsageThresh, UInt8 patternsCnt, const gpCom_FilterPattern_t* patterns)
{
    UInt8 i, j;
    if(!patterns)
    {
        return false;
    }

    // Find the position of the filter to be removed
    for(i = 0; i < GP_COM_FILTER_CNT; i++)
    {
        if(filters[i].enabled && filters[i].moduleId == moduleId && filters[i].bufferUsageThresh == bufferUsageThresh && patternsCnt == filters[i].patternsCnt && isPatternEqual(patterns, filters[i].patterns, filters[i].patternsCnt))
        {
            break;
        }
    }

    // If the filter was found, remove it and shift elements
    if(i < GP_COM_FILTER_CNT - 1)
    {
        for(j = i; j < GP_COM_FILTER_CNT - 1; j++)
        {
            filters[j] = filters[j + 1];
            if(j == GP_COM_FILTER_CNT - 2)
            {
                filters[GP_COM_FILTER_CNT - 1].enabled = false;
            }
        }
    }
    else
    {
        if(i == GP_COM_FILTER_CNT - 1)
        {
            filters[i].enabled = false;
            return true;
        }
        return false;
    }
    return true;
}

Bool gpCom_FilterIsPacketToReject(gpCom_Packet_t* pPacket, UInt8 queueUsagePer)
{
    UInt8 filterIdx;
    UInt8 patternIdx;

    Bool patternFound = false;

    for(filterIdx = 0; (filterIdx < GP_COM_FILTER_CNT) && !patternFound; filterIdx++)
    {
        patternFound = false;
        // Check if filter is enabled and the moduleId is match:
        if(!filters[filterIdx].enabled || (filters[filterIdx].moduleId != pPacket->moduleID))
        {
            continue;
        }

        // Check the queue usage threshold (if set to 0, ignore the usage and go to the next step):
        if(filters[filterIdx].bufferUsageThresh && (queueUsagePer <= filters[filterIdx].bufferUsageThresh))
        {
            continue;
        }

        // Check if all of the patterns are found in the packet data:
        for(patternIdx = 0; patternIdx < filters[filterIdx].patternsCnt; patternIdx++)
        {
            patternFound = isFilterPatternInPacket(&filters[filterIdx].patterns[patternIdx], pPacket);

            if(!patternFound)
            {
                break; // Pattern not found, break the loop and skip to the next filter
            }
        }
    }

    return patternFound;
}

static Bool isFilterPatternInPacket(gpCom_FilterPattern_t* pPattern, gpCom_Packet_t* pPacket)
{
    // Check if the packet is long enough to apply this filtering pattern:
    if((pPattern->offset + pPattern->len) > pPacket->length)
    {
        return false;
    }

    // Compare pattern data with the packet data on given offset:
    if(!memcmp(&pPacket->packet[pPattern->offset], pPattern->data, pPattern->len))
    {
        return true;
    }

    return false;
}

static UInt8 getFilterIdx(UInt8 threshold)
{
    UInt8 i;
    int j;

    // Find the position where filter with the threshold should be inserted
    for(i = 0; i < GP_COM_FILTER_CNT; i++)
    {
        if(!filters[i].enabled || (filters[i].bufferUsageThresh > threshold))
        {
            break;
        }
    }
    for(j = GP_COM_FILTER_CNT - 1; j >= i; j--)
    {
        if(filters[i].enabled)
        {
            filters[j + 1] = filters[j];
        }
    }
    return i;
}

static Bool isPatternEqual(const gpCom_FilterPattern_t* pInPattern, const gpCom_FilterPattern_t* pOutPattern, UInt8 patternsCnt)
{
    int i;
    for(i = 0; i < patternsCnt; i++)
    {
        if(pInPattern[i].len == pOutPattern[i].len && pInPattern[i].offset == pOutPattern[i].offset)
        {
            if(memcmp(pInPattern[i].data, pOutPattern[i].data, pOutPattern[i].len))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}
#endif //def GP_COM_DIVERSITY_PACKET_FILTERING
