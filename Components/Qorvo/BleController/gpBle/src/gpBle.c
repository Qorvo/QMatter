/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
 *
 *   Bluetooth Low Energy interface
 *   Implementation of gpBle
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

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLE

#include "gpHci_Includes.h"
#include "gpBle.h"
#include "gpBle_defs.h"
#include "gpBleActivityManager.h"
#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
#include "gpBleDataRx.h"
#endif
#include "gpLog.h"
#include "gpSched.h"
#include "gpPoolMem.h"
#include "gpAssert.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_NR_OF_SUPPORTED_COMMAND_CREDITS     1


#define BLE_BUFFERTYPE_SOLICITED_INDEX_START            (0)
#define BLE_BUFFERTYPE_SOLICITED_INDEX_END              (BLE_BUFFERTYPE_SOLICITED_INDEX_START + GP_BLE_NR_OF_SOLICITED_EVENT_BUFFERS)
#define BLE_BUFFERTYPE_UNSOLICITED_INDEX_START          (BLE_BUFFERTYPE_SOLICITED_INDEX_END)
#define BLE_BUFFERTYPE_UNSOLICITED_INDEX_END            (BLE_BUFFERTYPE_UNSOLICITED_INDEX_START + GP_BLE_NR_OF_UNSOLICITED_EVENT_BUFFERS)
#define BLE_BUFFERTYPE_CONNECTION_COMPLETE_INDEX_START  (BLE_BUFFERTYPE_UNSOLICITED_INDEX_END)
#define BLE_BUFFERTYPE_CONNECTION_COMPLETE_INDEX_END    (BLE_BUFFERTYPE_CONNECTION_COMPLETE_INDEX_START + GP_BLE_NR_OF_CONNECTION_COMPLETE_EVENT_BUFFERS)


/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_IS_COMMAND_COMPLETE_PENDING()           (Ble_EventBuffers[BLE_BUFFERTYPE_SOLICITED_INDEX_START] != NULL)

// If number of solicited event buffers is different from 1, some functionality in this module will break
// See BLE_IS_COMMAND_COMPLETE_PENDING and gpBle_AllocateEventBuffer
GP_COMPILE_TIME_VERIFY(GP_BLE_NR_OF_SOLICITED_EVENT_BUFFERS == 1);
GP_COMPILE_TIME_VERIFY((BLE_BUFFERTYPE_SOLICITED_INDEX_END - BLE_BUFFERTYPE_SOLICITED_INDEX_START) == 1);

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

// The user for all advertising related activities. Once a user is selected, this cannot be changed without a reset.
#define gpBle_AdvertisingUserNone       0x00
#define gpBle_AdvertisingUserLegacy     0x01
#define gpBle_AdvertisingUserExtended   0x02
typedef UInt8 gpBle_AdvertisingUser_t;

// Common properties of a BLE event buffer type
typedef struct {
    UInt8 startIndex;
    UInt8 endIndex;
    UInt16 size;
    Ble_EventBufferType_t bufferType;
} Ble_BufferTypeMapping_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

// Fixed buffer for sollicted events (command complete/command status)
// We only allow a maximum of 1 solicited buffers at any time.
// Use a fixed buffer, as we want to guarantee this event is sent when there are no more available poolmems
static gpBle_EventBuffer_t Ble_SollicitedEventBuffer;
static gpBle_EventBuffer_t* Ble_EventBuffers[GP_BLE_NR_OF_EVENT_BUFFERS];
static gpBle_EventServer_t Ble_EventBufferServer[GP_BLE_NR_OF_EVENT_BUFFERS];

static const Ble_BufferTypeMapping_t Ble_BufferTypeMappings[] =
{
    {.startIndex = BLE_BUFFERTYPE_SOLICITED_INDEX_START, .endIndex = BLE_BUFFERTYPE_SOLICITED_INDEX_END, .size = sizeof(gpBle_EventBuffer_t), .bufferType = Ble_EventBufferType_Solicited},
    {.startIndex = BLE_BUFFERTYPE_UNSOLICITED_INDEX_START, .endIndex = BLE_BUFFERTYPE_UNSOLICITED_INDEX_END, .size = sizeof(gpBle_EventBuffer_t), .bufferType = Ble_EventBufferType_Unsolicited},
    {.startIndex = BLE_BUFFERTYPE_CONNECTION_COMPLETE_INDEX_START, .endIndex = BLE_BUFFERTYPE_CONNECTION_COMPLETE_INDEX_END, .size = GPBLE_BUFFERSIZE_CONNECTION_COMPLETE, .bufferType = Ble_EventBufferType_ConnectionComplete},
};


/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static gpBle_EventServer_t Ble_GetCommandServer(gpBle_EventServer_t eventServer,gpHci_CommandOpCode_t opCode);

// event communication
static void Ble_FreeEventBuffer(gpBle_EventBufferHandle_t eventBufferHandle);
static void Ble_SendEventBufferToHci(gpBle_EventBuffer_t* pEventBuf);
static void Ble_SendEventBufferToHciWrapper(void* pArg);


#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
static void Ble_cbConnEventDone(Ble_IntConnId_t connId);
#endif
static gpBle_EventBuffer_t* Ble_AllocateEventBuffer(const Ble_BufferTypeMapping_t* pMapping);
static const Ble_BufferTypeMapping_t* Ble_GetEventBufferMapping(gpBle_EventBufferHandle_t eventBufferHandle);
static UInt16 Ble_GetEventBufferSize(gpBle_EventBufferHandle_t eventBufferHandle);
static Ble_EventBufferType_t Ble_GetEventBufferType(gpBle_EventBufferHandle_t eventBufferHandle);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

gpBle_EventServer_t Ble_GetCommandServer(gpBle_EventServer_t eventServer,gpHci_CommandOpCode_t opCode)
{
#if defined(GP_DIVERSITY_BLE_EXECUTE_CMD_HCIWRAPPER) && defined(GP_DIVERSITY_BLE_EXECUTE_CMD_WCBLEHOST)
    switch (opCode)
    {
        case gpHci_OpCodeLeReceiverTest:
        case gpHci_OpCodeLeTransmitterTest:
        case gpHci_OpCodeLeEnhancedReceiverTest:
        case gpHci_OpCodeLeEnhancedTransmitterTest:
        case gpHci_OpCodeLeTestEnd:
        case gpHci_OpCodeVsdSetTransmitPower:
        case gpHci_OpCodeSetVsdTestParams:
        case gpHci_OpCodeReset:
        case gpHci_OpCodeVsdWriteDeviceAddress:
           return eventServer; /* All servers can execute these commands */
        default:
           return gpBle_EventServer_Host;
    }
#elif defined(GP_DIVERSITY_BLE_EXECUTE_CMD_HCIWRAPPER)
    return gpBle_EventServer_Wrapper;
#elif defined(GP_DIVERSITY_BLE_EXECUTE_CMD_WCBLEHOST)
    return gpBle_EventServer_Host;
#else
#error
#endif
}

void Ble_FreeEventBuffer(gpBle_EventBufferHandle_t eventBufferHandle)
{
    Ble_EventBufferType_t bufferType = Ble_GetEventBufferType(eventBufferHandle);
    gpBle_EventBuffer_t* pBuf;

    if(eventBufferHandle >= GP_BLE_NR_OF_EVENT_BUFFERS || bufferType == Ble_EventBufferType_Invalid)
    {
        GP_LOG_SYSTEM_PRINTF("Error: index %u or type %u not valid!",0, eventBufferHandle, bufferType);
        GP_ASSERT_DEV_EXT(false);
        return;
    }

    pBuf = Ble_EventBuffers[eventBufferHandle];

    if (pBuf == NULL)
    {
        GP_LOG_SYSTEM_PRINTF("Error: event buffer %u has no valid ptr!",0, eventBufferHandle);
        GP_ASSERT_DEV_EXT(false);
        return;
    }

    if(bufferType != Ble_EventBufferType_Solicited)
    {
        // If not a solicited buffer, we need to free the poolmem
        gpPoolMem_Free(pBuf);
    }

    Ble_EventBuffers[eventBufferHandle] = NULL;
}

void Ble_SendEventBufferToHci(gpBle_EventBuffer_t* pEventBuf)
{
    gpBle_EventBufferInfo_t eventBufferInfo;

    MEMSET(&eventBufferInfo, 0, sizeof(gpBle_EventBufferInfo_t));

    eventBufferInfo.eventHandle = gpBle_EventBufferToHandle(pEventBuf);

    if(eventBufferInfo.eventHandle != GP_BLE_EVENT_HANDLE_IDX_INVALID)
    {
        eventBufferInfo.eventServer = Ble_EventBufferServer[eventBufferInfo.eventHandle];

        if(eventBufferInfo.eventServer == gpBle_EventServer_Invalid)
        {
#if defined(GP_DIVERSITY_BLE_EXECUTE_CMD_WCBLEHOST)
            eventBufferInfo.eventServer = gpBle_EventServer_Host;
#elif defined(GP_DIVERSITY_BLE_EXECUTE_CMD_HCIWRAPPER)
            eventBufferInfo.eventServer = gpBle_EventServer_Wrapper;
#endif
        }
    }

    gpBle_cbEventIndication(&eventBufferInfo);
}

void Ble_SendEventBufferToHciWrapper(void* pArg)
{
    gpBle_EventBuffer_t* pEventBuf = (gpBle_EventBuffer_t*)pArg;

    Ble_SendEventBufferToHci(pEventBuf);
}


#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
/*
 * This function implements a fixed priority mechanism to notify services about the end of a connection event.
 * For now this mechanism is Data_Rx, Connections monitor, LLCP
*/
void Ble_cbConnEventDone(Ble_IntConnId_t connId)
{
    GP_LOG_PRINTF("conn ev done: %x",0, connId);

    gpBle_DataRxConnEventDone(connId);
    gpBleActivityManager_ConnectionEventDone(connId);
}
#endif //GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE

gpBle_EventBuffer_t* Ble_AllocateEventBuffer(const Ble_BufferTypeMapping_t* pMapping)
{
    UIntLoop i;

    if(pMapping == NULL)
    {
        return NULL;
    }

    for(i = pMapping->startIndex; i < pMapping->endIndex; i++)
    {
        if(Ble_EventBuffers[i] == NULL)
        {
            if(pMapping->bufferType == Ble_EventBufferType_Solicited)
            {
                Ble_EventBuffers[i] = &Ble_SollicitedEventBuffer;
            }
            else
            {
                Ble_EventBuffers[i] = (gpBle_EventBuffer_t*) GP_POOLMEM_TRYMALLOC(pMapping->size);
            }

            if(Ble_EventBuffers[i] == NULL)
            {
                // We failed to allocate a poolmem
                GP_LOG_SYSTEM_PRINTF("Warning: could not allocate poolmem for HCI event buffer type %u",0, pMapping->bufferType);
                return NULL;
            }

            MEMSET(Ble_EventBuffers[i], 0, pMapping->size);
            Ble_EventBufferServer[i] = gpBle_EventServer_Invalid;
            return Ble_EventBuffers[i];
        }
    }

    if(pMapping->bufferType == Ble_EventBufferType_ConnectionComplete)
    {
        GP_LOG_SYSTEM_PRINTF("Error: no room to store buffer for conn complete.",0);
        GP_ASSERT_DEV_EXT(false);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("Warning: all event buffers taken for buffer type %u",0, pMapping->bufferType);
    }

    return NULL;
}

const Ble_BufferTypeMapping_t* Ble_GetEventBufferMapping(gpBle_EventBufferHandle_t eventBufferHandle)
{
    UIntLoop i;

    if(eventBufferHandle >= GP_BLE_NR_OF_EVENT_BUFFERS)
    {
        return NULL;
    }

    for(i = 0; i < number_of_elements(Ble_BufferTypeMappings); i++)
    {
        if(BLE_RANGE_CHECK(eventBufferHandle, Ble_BufferTypeMappings[i].startIndex, Ble_BufferTypeMappings[i].endIndex-1))
        {
            return &Ble_BufferTypeMappings[i];
        }
    }

    return NULL;
}

UInt16 Ble_GetEventBufferSize(gpBle_EventBufferHandle_t eventBufferHandle)
{
    const Ble_BufferTypeMapping_t* pMapping = Ble_GetEventBufferMapping(eventBufferHandle);

    if(pMapping == NULL)
    {
        return 0;
    }

    return pMapping->size;
}

Ble_EventBufferType_t Ble_GetEventBufferType(gpBle_EventBufferHandle_t eventBufferHandle)
{
    const Ble_BufferTypeMapping_t* pMapping = Ble_GetEventBufferMapping(eventBufferHandle);

    if(pMapping == NULL)
    {
        return Ble_EventBufferType_Invalid;
    }

    return pMapping->bufferType;
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBle_Init(gpHal_BleCallbacks_t* pCallbacks)
{
    UIntLoop i;

    // We do not allow more than 1 solicited event buffer!
    COMPILE_TIME_ASSERT(GP_BLE_NR_OF_SOLICITED_EVENT_BUFFERS == 1);


    MEMSET(&Ble_SollicitedEventBuffer, 0, sizeof(gpBle_EventBuffer_t));

    for(i = 0; i < GP_BLE_NR_OF_EVENT_BUFFERS; i++)
    {
        Ble_EventBuffers[i] = NULL;
    }

#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
    gpBle_ConnectionsInit();
    pCallbacks->cbConnEventDone = Ble_cbConnEventDone;
#else
    pCallbacks->cbConnEventDone = NULL;
#endif // GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE

}

void gpBle_ResetBlock(Bool firstReset)
{
#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
    gpBle_ConnectionsReset(firstReset);
#endif // GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE
}

gpBle_EventBuffer_t* gpBle_AllocateEventBuffer(Ble_EventBufferType_t bufferType)
{
    UIntLoop i;
    gpBle_EventBuffer_t* pEventBuf = NULL;

    if(bufferType == Ble_EventBufferType_Solicited && Ble_EventBuffers[BLE_BUFFERTYPE_SOLICITED_INDEX_START] != NULL)
    {
        // Buffer is not available ==> indicates we are triggering 2 solicted events at the same time
        GP_LOG_SYSTEM_PRINTF("Error: solicited HCI event buffer already taken!",0);
        GP_ASSERT_DEV_EXT(false);
        return NULL;
    }

    for(i = 0; i < number_of_elements(Ble_BufferTypeMappings); i++)
    {
        const Ble_BufferTypeMapping_t* pMapping = &Ble_BufferTypeMappings[i];

        if(pMapping->bufferType == bufferType)
        {
            pEventBuf = Ble_AllocateEventBuffer(pMapping);
            break;
        }
    }

    return pEventBuf;
}

gpBle_EventBufferHandle_t gpBle_EventBufferToHandle(gpBle_EventBuffer_t *pEventBuffer)
{
    UIntLoop i;
    gpBle_EventBufferHandle_t handle = GP_BLE_EVENT_HANDLE_IDX_INVALID;

    for(i = 0; i < GP_BLE_NR_OF_EVENT_BUFFERS; i++)
    {
        if(Ble_EventBuffers[i] == pEventBuffer)
        {
            handle = i;
            break;
        }
    }

    return handle;
}

gpBle_EventBuffer_t * gpBle_EventHandleToBuffer(gpBle_EventBufferHandle_t eventHandle)
{
    gpBle_EventBuffer_t* pBuf = NULL;

    if(eventHandle < GP_BLE_NR_OF_EVENT_BUFFERS)
    {
        pBuf = Ble_EventBuffers[eventHandle];
    }

    return pBuf;
}

void gpBle_GetEventCodes(gpBle_EventBufferHandle_t eventHandle, gpHci_EventCode_t *eventCode, gpHci_LEMetaSubEventCode_t *subEventCode, gpHci_ConnectionHandle_t *connectionHandle)
{
    *eventCode = 0;
    *subEventCode = 0;
    *connectionHandle = BLE_CONN_HANDLE_INVALID;

    gpBle_EventBuffer_t * pBuf = gpBle_EventHandleToBuffer(eventHandle);

    if(pBuf)
    {
        *eventCode = pBuf->eventCode;
        if(pBuf->eventCode == gpHci_EventCode_LEMeta)
        {
            *subEventCode = pBuf->payload.metaEventParams.subEventCode;
        }
    }
}

void gpBle_CopyEventBuffer(gpBle_EventBuffer_t *pEventBuffer, gpBle_EventBufferHandle_t eventHandle)
{
    if(GP_BLE_EVENT_HANDLE_ISVALID(eventHandle))
    {
        UInt16 bufferSize = Ble_GetEventBufferSize(eventHandle);

        MEMCPY(pEventBuffer, Ble_EventBuffers[eventHandle], bufferSize);
    }
    else
    {
        if(pEventBuffer != NULL)
        {
            pEventBuffer->eventCode = gpHci_EventCode_Nothing;
        }
    }
}

void gpBle_ExecuteCommand(gpBle_EventServer_t eventServer, gpHci_CommandOpCode_t opCode, gpBle_ActionFunc_t actionFunc, gpHci_CommandParameters_t* pParams)
{
    gpHci_Result_t result = gpHci_ResultInvalid;
    gpBle_EventBuffer_t* pEventBuf;
    gpBle_EventBufferHandle_t eventHandle;
    gpBle_EventServer_t opCode_eventServer;

    if(BLE_IS_COMMAND_COMPLETE_PENDING())
    {
        return;
    }

    GP_ASSERT_DEV_EXT(pParams != NULL);
    GP_ASSERT_DEV_EXT(actionFunc != NULL);

    pEventBuf = gpBle_AllocateEventBuffer(Ble_EventBufferType_Solicited);

    GP_ASSERT_DEV_EXT(pEventBuf != NULL);

    // Set invalid response event, each service should set an appropriate event
    BLE_SET_RESPONSE_EVENT_INVALID(pEventBuf->eventCode);

    opCode_eventServer = Ble_GetCommandServer(eventServer, opCode);

    GP_LOG_PRINTF("Execute: %x %x->%x", 0, eventServer, opCode, opCode_eventServer);

    {
        if (eventServer == opCode_eventServer)
        {
            result = actionFunc(pParams, pEventBuf);
        }
        else /* The executing server is not allowed to execute this command */
        {
            GP_LOG_SYSTEM_PRINTF("Server: %x may not execute op %x", 0, eventServer, opCode);
            result = gpBle_UnknownOpCode(pParams, pEventBuf);
        }
    }

    eventHandle = gpBle_EventBufferToHandle(pEventBuf);

    /* We store the server that executed the command. */
    Ble_EventBufferServer[eventHandle] = eventServer;

    GP_ASSERT_DEV_EXT(BLE_IS_RESPONSE_EVENT_VALID(pEventBuf->eventCode));

    Ble_EventSpecificHandling(pEventBuf, opCode, result);

    if(BLE_IS_RESPONSE_EVENT_NEEDED(pEventBuf->eventCode))
    {
        Ble_SendEventBufferToHci(pEventBuf);
    }
    else
    {
        // Free the buffer
        Ble_FreeEventBuffer(eventHandle);
    }
}

void gpBle_SendVsdMetaEvent(gpHci_VsdMetaEventParams_t* pParams)
{
    gpBle_EventBuffer_t* pEventBuf;

    pEventBuf = gpBle_AllocateEventBuffer(Ble_EventBufferType_Unsolicited);
    GP_ASSERT_DEV_EXT(pEventBuf != NULL);

    pEventBuf->eventCode = gpHci_EventCode_VsdMeta;
    MEMCPY(&pEventBuf->payload.VsdMetaEventParams, pParams, sizeof(gpHci_VsdMetaEventParams_t));

    Ble_SendEventBufferToHci(pEventBuf);
}

void gpBle_SendCommandCompleteEvent(gpHci_CommandCompleteParams_t* params)
{
    gpBle_EventBuffer_t* pEventBuf;

    pEventBuf = gpBle_AllocateEventBuffer(Ble_EventBufferType_Unsolicited);
    GP_ASSERT_DEV_EXT(pEventBuf != NULL);

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    MEMCPY(&pEventBuf->payload.commandCompleteParams, params, sizeof(gpHci_CommandCompleteParams_t));

    Ble_SendEventBufferToHci(pEventBuf);
}

void gpBle_SendEvent(gpBle_EventBuffer_t* pEventBuf)
{
    Ble_SendEventBufferToHci(pEventBuf);
}

Bool gpBle_SendEventWithPayload(gpHci_EventCode_t eventCode, gpHci_EventCbPayload_t* pPayload)
{
    gpBle_EventBuffer_t* pEventBuf;

    pEventBuf = gpBle_AllocateEventBuffer(Ble_EventBufferType_Unsolicited);

    if(pEventBuf == NULL)
    {
        return false;
    }

    pEventBuf->eventCode = eventCode;
    MEMCPY(&pEventBuf->payload, pPayload, sizeof(gpHci_EventCbPayload_t));

    Ble_SendEventBufferToHci(pEventBuf);

    return true;
}

Bool gpBle_ScheduleEvent(UInt32 delayUs, gpHci_EventCode_t eventCode, gpHci_EventCbPayload_t* pPayload)
{
    gpBle_EventBuffer_t* pEventBuf;

    pEventBuf = gpBle_AllocateEventBuffer(Ble_EventBufferType_Unsolicited);

    if(pEventBuf == NULL)
    {
        GP_ASSERT_DEV_EXT(false);
        return false;
    }

    pEventBuf->eventCode = eventCode;
    MEMCPY(&pEventBuf->payload, pPayload, sizeof(gpHci_EventCbPayload_t));

    // Schedule the execution
    gpSched_ScheduleEventArg(delayUs, Ble_SendEventBufferToHciWrapper, (void*)pEventBuf);

    return true;
}

void gpBle_HciEventConfirm(gpBle_EventBufferHandle_t eventBufferHandle)
{
    Ble_FreeEventBuffer(eventBufferHandle);
}
