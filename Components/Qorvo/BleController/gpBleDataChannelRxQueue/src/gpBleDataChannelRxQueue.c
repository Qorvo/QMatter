/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
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

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLEDATACHANNELRXQUEUE

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBleConfig.h"
#include "gpBleDataChannelRxQueue.h"
#include "gpBleSecurityCoprocessor.h"
#include "gpBleDataRx.h"
#include "gpBleLlcp.h"
#include "gpBleLlcpFramework.h"
#include "gpBle_defs.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpBle_LLCP_getters.h"
#include "gpLog.h"


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define BLE_HAL_DATA_IND_FLOW_GO 0x0000
#define BLE_HAL_DATA_IND_FLOW_STOP 0xFFFF

#define BLE_AUTH_PAYLOAD_MAX_OFFSET_FOR_SMALL_INTERVALS_US  1000000UL

#define BLE_RX_QUEUE_SIZE                             GP_HAL_NR_OF_RX_PBMS
#define BLE_IS_FLOW_STOPPED(connMask)                 (connMask!=0)

#define BLE_MIN_PING_MARGIN                           3
#define BLE_PING_MARGIN                               (BLE_MIN_PING_MARGIN + 2)
#define BLE_PING_ADVANCED_WAKEUP                      2000

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

#define BLE_RX_GET_NEARLY_AUTH_TO_UNIT(authTo, offset)  ((authTo > offset) ? (authTo - offset) : (authTo))

#define BLE_ENCRYPTION_ENABLE(connId)       (Ble_DataRxAttributes.encryptionEnabled |= (1 << connId))
#define BLE_ENCRYPTION_DISABLE(connId)      (Ble_DataRxAttributes.encryptionEnabled &= ~(1 << connId))
#define BLE_ENCRYPTION_IS_ENABLED(connId)   ((Ble_DataRxAttributes.encryptionEnabled & (1 << connId)) != 0)

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    UInt8 connId;
    UInt16 authPayloadToUnit;
} Ble_RxLinkContex_t;

typedef struct {
    UInt16 encryptionEnabled;   // bit field: one per connection
    UInt16 rxFlowCtrl;          // bit field: one per connection
} Ble_DataChannelRxAttributes_t;

typedef struct {
    Ble_IntConnId_t connId;
    gpPd_Loh_t pdLoh;
} Ble_PDUData_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
// Context per link
static Ble_RxLinkContex_t      Ble_RxLinkContext[BLE_LLCP_MAX_NR_OF_CONNECTIONS];
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED

// Global context
static Ble_DataChannelRxAttributes_t Ble_DataRxAttributes;
// Queue related data
static UInt16 Ble_QueueFlowConnMask = 0;

static Ble_PDUData_t Ble_PDUQueue[BLE_RX_QUEUE_SIZE];
static volatile UInt8 Ble_PDUQueue_head = 0;
static volatile UInt8 Ble_PDUQueue_size = 0;

#ifdef GP_DIVERSITY_DEVELOPMENT

static Bool gpBle_VsdAuthenticatedPayloadTimeoutEnable;

static UInt32 gpBle_VsdDataChannelRxQueueLatency = 0;
typedef struct Delayed_HalDataInd_ {
    UInt8 connId;
    gpPd_Loh_t pdLoh;
} Delayed_HalDataInd_t;
static UInt8 delayedQueueHead = BLE_RX_QUEUE_SIZE-1;
static UInt8 delayedQueueTail = 0;
static Delayed_HalDataInd_t delayedQueue[BLE_RX_QUEUE_SIZE];
#endif /* GP_DIVERSITY_DEVELOPMENT */

#if defined(GP_DIVERSITY_LOG) && defined(GP_LOCAL_LOG)
static UInt32 T_FlowStop;
static UInt32 T_FlowStop_2;
#endif /* GP_LOCAL_LOG */

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/


#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
static void Ble_DataRxQueueEnableAuthPayloadTimers(Ble_IntConnId_t connId, Bool enable);
static void Ble_DataRxQueueAuthPayloadNearlyExpired(void* pArg);
static void Ble_DataRxQueueAuthPayloadExpired(void* pArg);
static void Ble_DataRxResetAuthPayloadTimers(Ble_IntConnId_t connId);
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED

static void Ble_HalDataInd(UInt8 connId, gpPd_Loh_t pdLoh);
static void Ble_ProcessHalDataInd(UInt8 connId, gpPd_Loh_t pdLoh);

static void Ble_PDUQueuePop(void);
static UInt8 Ble_PDUQueueAlloc(void);

#ifdef GP_DIVERSITY_DEVELOPMENT
static void Ble_HandleDelayedDataInd(void *);
static void Ble_HalDataInd_real(UInt8 connId, gpPd_Loh_t pdLoh);
#endif // GP_DIVERSITY_DEVELOPMENT

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
static gpHal_Result_t Ble_DataRxSetFlowBitsForDataInd(UInt16 connMask)
{
    Bool result  = gpHal_ResultSuccess;

    if (Ble_DataRxAttributes.rxFlowCtrl != connMask)
    {
        result = gpHal_BleSetFlowCtrl(connMask);
        GP_ASSERT_SYSTEM(result == gpHal_ResultSuccess);
        Ble_DataRxAttributes.rxFlowCtrl = connMask;

#if defined(GP_DIVERSITY_LOG) && defined(GP_LOCAL_LOG)
        {
            UInt32 Timestamp;
            gpHal_GetTime(&Timestamp);

            if (BLE_IS_FLOW_STOPPED(connMask))
            {
                T_FlowStop = Timestamp;
                GP_LOG_PRINTF("T=%lu : HalDataInd: flow STOP",0,(unsigned long)Timestamp);
            }
            else
            {
                GP_ASSERT_SYSTEM(Timestamp >=  T_FlowStop);
                GP_LOG_PRINTF("T=%lu : HalDataInd: flow GO: DeltaT=%lu",0,(unsigned long)Timestamp, (unsigned long)(Timestamp - T_FlowStop));
            }
        }
#endif /* GP_DIVERSITY_LOG && GP_LOCAL_LOG */
    }

    return result;
}

static void Ble_CheckAndUpdateFlowStopForDataInd(UInt16 connMask, UInt8 QSize)
{
    GP_ASSERT_SYSTEM(connMask != 0);
    if ( QSize >= (2*BLE_RX_QUEUE_SIZE)/3 )
    {
        /* We already stop the flow if the queue is almost full.
         * We don't want the queue to be full because this
         * would mean all PBM's are being used for data. */
        Ble_DataRxSetFlowBitsForDataInd(connMask);
    }
}

static void Ble_CheckAndUpdateFlowGoForDataInd(UInt16 connMask, UInt8 QSize)
{
    GP_ASSERT_SYSTEM(connMask == 0);
    if ( QSize <= BLE_RX_QUEUE_SIZE/3 )
    {
        /* We already enable the flow control when the queue is almost empty, this
         * prevents the dataRX system from having to wait a long time for data
         * after enabling flow control (especially true for large packets). */
        Ble_DataRxSetFlowBitsForDataInd(connMask);
    }
}

static void Ble_PDUQueuePop(void)
{
    Ble_PDUData_t pduDesc;
    UInt8 oldHead = BLE_RX_QUEUE_SIZE;
    UInt8 newSize=0;

    MEMSET(&pduDesc, 0, sizeof(Ble_PDUData_t));

    if(BLE_IS_FLOW_STOPPED(Ble_QueueFlowConnMask))
    {
        /* If the queue is empty but Ble_ProcessHalDataInd is
         * interrupted by a new pdu before the flow is stopped
         * this function will be run during flow stop and must return. */
        return;
    }

    /* TODO: only disable data indication interrupts! */
    HAL_DISABLE_GLOBAL_INT ();
    {
        /* We don't want to get interrupted by data indication
         * interrupts as this might cause concurreny issues. */
        if (Ble_PDUQueue_size)
        {
            oldHead=Ble_PDUQueue_head;
            pduDesc = Ble_PDUQueue[oldHead];
            if (++Ble_PDUQueue_head==BLE_RX_QUEUE_SIZE)
            {
                Ble_PDUQueue_head=0;
            }
            newSize=--Ble_PDUQueue_size;
        }
    }
    HAL_ENABLE_GLOBAL_INT();

    if(oldHead==BLE_RX_QUEUE_SIZE)
    {
        /* If a reset happens after scheduling of this
         * we need to return.*/
        return;
    }
    Ble_ProcessHalDataInd(pduDesc.connId,pduDesc.pdLoh);
    if (!BLE_IS_FLOW_STOPPED(Ble_QueueFlowConnMask)) // Note that Data Rx Service may have changed the Ble_QueueFlowConnMask
    {
        if (newSize>0)
        {
            gpSched_ScheduleEvent(0, Ble_PDUQueuePop);
        }
    }
    Ble_CheckAndUpdateFlowGoForDataInd(BLE_HAL_DATA_IND_FLOW_GO, newSize);
}

static UInt8 Ble_PDUQueueAlloc(void)
{
    UInt8 tail = BLE_RX_QUEUE_SIZE;
    if (Ble_PDUQueue_size < BLE_RX_QUEUE_SIZE)
    {
        tail = Ble_PDUQueue_head;
        tail += Ble_PDUQueue_size;
        tail %= BLE_RX_QUEUE_SIZE;

        if (0==Ble_PDUQueue_size && !BLE_IS_FLOW_STOPPED(Ble_QueueFlowConnMask))
        {
            gpSched_ScheduleEvent(0, Ble_PDUQueuePop);
        }

        ++Ble_PDUQueue_size;
    }
    return tail;
}

void Ble_ProcessHalDataInd(UInt8 connId, gpPd_Loh_t pdLoh)
{
    Ble_DataChannelHeader_t header;

    Ble_ParseDataChannelPduHeader(&pdLoh, &header);

    if(header.llid == Ble_LLID_Reserved)
    {
        // Drop PDUs with reserved llid (recommendation from the spec Vol 1 Part E $2.4)
        // Where a field, parameter, or other variable object can take a range of values and some values are described as
        // "Reserved for future use", devices shall not set the object to any of those values. A device receiving
        // an object with such a value should reject it.
        GP_LOG_PRINTF("Drop PDU with reserved llid",0);
        Ble_RMFreeResource(connId, pdLoh.handle);
        return;
    }

    if(BLE_ENCRYPTION_IS_ENABLED(connId) && RANGE_CHECK(header.length, 1, BLE_SEC_MIC_LENGTH))
    {
        // Vol 6 Part E $1: on an encrypted link all PDUs with payload length > 0 shall be encrypted and authenticated
        // Since we need at least 4 bytes for the MIC, all packets with length > 0 and <= 4 are invalid
        GP_LOG_SYSTEM_PRINTF("Non-encrypted PDU on encrypted link: disconnect",0);
        gpBle_StopConnection(gpBleLlcp_IntHandleToHciHandle(connId), gpHci_ResultConnectionTerminatedduetoMICFailure);
        Ble_RMFreeResource(connId, pdLoh.handle);
        return;
    }


    if(header.length == 0)
    {
        // PDUs with length 0 are not useful anymore at this point.
        // This can be an empty PDU with CTE (in which case the CTE part is already processed above),
        // Or an invalid packet (e.g: a control packet with length 0). Drop it here.
        GP_LOG_PRINTF("Drop zero-length PDU",0);
        Ble_RMFreeResource(connId, pdLoh.handle);
        return;
    }

#ifdef GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED
    if(BLE_ENCRYPTION_IS_ENABLED(connId))
    {
        gpHci_Result_t result;

        // We need to zero out MD, NESN and SN for decryption (these bits are not needed further on and can just be put to zero)
        UInt8 maskedHeaderByte = gpPd_ReadByte(pdLoh.handle, pdLoh.offset) & BLE_DATA_PDU_FIRST_HEADER_BYTE_AUTH_MASK;
        gpPd_WriteByte(pdLoh.handle, pdLoh.offset, maskedHeaderByte);

        result = gpBle_SecurityCoprocessorCcmDecryptAcl(connId, &pdLoh);

        if(result != gpHci_ResultSuccess)
        {
            // MIC failure, disconnect the link: cfr BT v4.2 Vol 6 Part E $1:
            // disconnect and do not send any further LLCP PDUs or data packets
            GP_LOG_SYSTEM_PRINTF("Decrypt failure res: %x ==> Disconnect!",0, result);
            gpBle_StopConnection(gpBleLlcp_IntHandleToHciHandle(connId), gpHci_ResultConnectionTerminatedduetoMICFailure);

            // drop the Pd
            Ble_RMFreeResource(connId, pdLoh.handle);
            return;
        }

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
#ifdef GP_DIVERSITY_DEVELOPMENT
        if (gpBle_VsdAuthenticatedPayloadTimeoutEnable)
#endif /* GP_DIVERSITY_DEVELOPMENT */
        {
          // restart the authenticated payload timeout
          Ble_DataRxQueueEnableAuthPayloadTimers(connId, true);
        }
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED

        // Successful decryption: update length field
        header.length -= BLE_SEC_MIC_LENGTH;
    }
#endif //GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED

    pdLoh.offset += BLE_PACKET_HEADER_SIZE;
    pdLoh.length -= BLE_PACKET_HEADER_SIZE;

#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    if(header.cp)
    {
        // Cut off cteInfo for higher layers
        pdLoh.offset += 1;
        pdLoh.length -= 1;
    }
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

    // Forward PDU to appropriate module (LLCP or data block)
    if(header.llid == Ble_LLID_Control)
    {
        // Forward to LLCP block
        Ble_LlcpDataInd(connId, pdLoh);
    }
    else
    {
        gpBle_DataRxIndication(connId, header.llid, pdLoh);
    }
}

/*****************************************************************************
 *                    HAL callback functions
 *****************************************************************************/

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
void Ble_DataRxQueueEnableAuthPayloadTimers(Ble_IntConnId_t connId, Bool enable)
{
    Ble_DataRxResetAuthPayloadTimers(connId);

    if(enable && gpBleConfig_IsEventMasked(gpHci_EventCode_AuthenticatedPayloadToExpired))
    {
        UInt32 nearlyExpiredToUs = 0;
        UInt32 authToUs = BLE_TIME_UNIT_10000_TO_US(Ble_RxLinkContext[connId].authPayloadToUnit);
        UInt32 minAuthToUs = gpBleLlcp_GetConnIntervalUs(connId) * (1 + (gpBleLlcp_IsMasterConnection(connId) ? gpBleLlcp_GetLatency(connId) : 0));

        if (authToUs > 2 * BLE_PING_MARGIN * minAuthToUs)
        {
            nearlyExpiredToUs = authToUs - BLE_PING_MARGIN * minAuthToUs;
        }
        else
        {
            minAuthToUs = BLE_PING_ADVANCED_WAKEUP + BLE_MIN_PING_MARGIN * minAuthToUs; /* +BLE_PING_ADVANCED_WAKEUP: wakeup a little in advance and prepare the Ping: send + receive and 1 retransmit */

            if (authToUs > minAuthToUs)
            {
                nearlyExpiredToUs = authToUs - minAuthToUs;
            }
            else
            {
                // Not enough time-margin for the LL Ping procedure: the best we can do is ping as often as possible
                nearlyExpiredToUs = 0;
            }
        }

        // Schedule timer for ping
        gpSched_ScheduleEventArg(nearlyExpiredToUs, Ble_DataRxQueueAuthPayloadNearlyExpired, &Ble_RxLinkContext[connId]);
        // Schedule timer for host notification
        gpSched_ScheduleEventArg(authToUs, Ble_DataRxQueueAuthPayloadExpired, &Ble_RxLinkContext[connId]);
    }
}

void Ble_DataRxQueueAuthPayloadNearlyExpired(void* pArg)
{
    Ble_RxLinkContex_t* pContext = (Ble_RxLinkContex_t*)pArg;
    gpBleLlcpFramework_StartProcedureDescriptor_t startDescriptor;

    GP_LOG_PRINTF("Auth payload TO nearly expired for conn: %x",0, pContext->connId);

    MEMSET(&startDescriptor, 0, sizeof(gpBleLlcpFramework_StartProcedureDescriptor_t));
    startDescriptor.procedureId = gpBleLlcp_ProcedureIdPing;
    startDescriptor.controllerInit = true;

    gpBleLlcpFramework_StartProcedure(pContext->connId, &startDescriptor);
}

void Ble_DataRxQueueAuthPayloadExpired(void* pArg)
{
    gpHci_EventCbPayload_t params;
    Ble_RxLinkContex_t* pContext = (Ble_RxLinkContex_t*)pArg;

    GP_LOG_PRINTF("Auth payload TO expired for conn: %x",0, pContext->connId);

    // Notify host
    params.authPayloadToExpired.connectionHandle = gpBleLlcp_IntHandleToHciHandle(pContext->connId);

    gpBle_ScheduleEvent(0, gpHci_EventCode_AuthenticatedPayloadToExpired, &params);

    // The timer shall be restarted after it has expired
    Ble_DataRxQueueEnableAuthPayloadTimers(pContext->connId, true);
}

void Ble_DataRxResetAuthPayloadTimers(Ble_IntConnId_t connId)
{
    // Disable previous running timers (if any)
    while(gpSched_UnscheduleEventArg(Ble_DataRxQueueAuthPayloadNearlyExpired, &Ble_RxLinkContext[connId])){}
    while(gpSched_UnscheduleEventArg(Ble_DataRxQueueAuthPayloadExpired, &Ble_RxLinkContext[connId])){}
}
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED

void Ble_HalDataInd(UInt8 connId, gpPd_Loh_t pdLoh)
{
#ifdef GP_DIVERSITY_DEVELOPMENT
    if (0 < gpBle_VsdDataChannelRxQueueLatency)
    {
        GP_ASSERT_DEV_INT(delayedQueueHead!=delayedQueueTail);
        delayedQueue[delayedQueueTail].connId = connId;
        delayedQueue[delayedQueueTail].pdLoh = pdLoh;
        gpSched_ScheduleEventArg(gpBle_VsdDataChannelRxQueueLatency,Ble_HandleDelayedDataInd,&delayedQueue[delayedQueueTail]);
        ++delayedQueueTail;
        delayedQueueTail = BLE_RX_QUEUE_SIZE <= delayedQueueTail ? 0 : delayedQueueTail;
    }
    else
    {
        Ble_HalDataInd_real(connId, pdLoh);
    }
}

void Ble_HandleDelayedDataInd(void *arg)
{
    UInt8 connId;
    gpPd_Loh_t pdLoh;
    Delayed_HalDataInd_t *delayedDataInd = (Delayed_HalDataInd_t*)arg;
    ++delayedQueueHead;
    delayedQueueHead = BLE_RX_QUEUE_SIZE <= delayedQueueHead ? 0 : delayedQueueHead;
    connId = delayedDataInd->connId;
    pdLoh = delayedDataInd->pdLoh;
    Ble_HalDataInd_real(connId, pdLoh);
}

void Ble_HalDataInd_real(UInt8 connId, gpPd_Loh_t pdLoh)
{
#endif // GP_DIVERSITY_DEVELOPMENT

    if(!BLE_IS_INT_CONN_HANDLE_VALID(connId))
    {
        GP_LOG_PRINTF("Received data from invalid conn id ==> drop",0);
        Ble_RMFreeResource(BLE_CONN_HANDLE_INVALID, pdLoh.handle);
        return;
    }

    if(pdLoh.length < BLE_PACKET_HEADER_SIZE)
    {
        // Valid packets should have at least 2 bytes header
        GP_LOG_PRINTF("Drop PDU with invalid header length",0);
        Ble_RMFreeResource(connId, pdLoh.handle);
        return;
    }

    {
        UInt8 queueIdx;
        queueIdx = Ble_PDUQueueAlloc();
        GP_ASSERT_SYSTEM(queueIdx<BLE_RX_QUEUE_SIZE);
        Ble_PDUQueue[queueIdx].connId = connId;
        Ble_PDUQueue[queueIdx].pdLoh  = pdLoh;
    }

    Ble_CheckAndUpdateFlowStopForDataInd(BLE_HAL_DATA_IND_FLOW_STOP, Ble_PDUQueue_size);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBle_DataRxQueueInit(gpHal_BleCallbacks_t* pCallbacks)
{
    // Assert when storage for connections is insufficient
    COMPILE_TIME_ASSERT(8*(sizeof(Ble_DataRxAttributes.encryptionEnabled)) >= BLE_LLCP_MAX_NR_OF_CONNECTIONS);
    COMPILE_TIME_ASSERT(8*(sizeof(Ble_DataRxAttributes.rxFlowCtrl)) >= BLE_LLCP_MAX_NR_OF_CONNECTIONS);
    COMPILE_TIME_ASSERT(BLE_PING_MARGIN >= BLE_MIN_PING_MARGIN);

    pCallbacks->cbDataInd = Ble_HalDataInd;
#ifdef GP_DIVERSITY_DEVELOPMENT
    gpBle_VsdAuthenticatedPayloadTimeoutEnable = true;
    gpBle_VsdDataChannelRxQueueLatency = 0;
#endif /* GP_DIVERSITY_DEVELOPMENT */
}

void gpBle_DataRxQueueReset(Bool firstReset)
{
    UIntLoop i;

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
    // Reset per-link context
    for(i = 0; i < BLE_LLCP_MAX_NR_OF_CONNECTIONS; i++)
    {
        Ble_RxLinkContext[i].connId = BLE_CONN_HANDLE_INVALID;
        Ble_RxLinkContext[i].authPayloadToUnit = GP_HCI_AUTHENTICATED_PAYLOAD_TO_DEFAULT;

        Ble_DataRxResetAuthPayloadTimers(i);
    }
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED

    // Reset global context
    Ble_DataRxAttributes.encryptionEnabled = (UInt16)0x00;
    Ble_DataRxAttributes.rxFlowCtrl = (UInt16)0x00;

    HAL_DISABLE_GLOBAL_INT();
    for (i=0;i<Ble_PDUQueue_size;++i)
    {
        UIntLoop idx = (Ble_PDUQueue_head+i)%BLE_RX_QUEUE_SIZE;
        gpPd_FreePd(Ble_PDUQueue[idx].pdLoh.handle);
    }
    Ble_PDUQueue_head = 0;
    Ble_PDUQueue_size = 0;
    Ble_QueueFlowConnMask = 0;
    HAL_ENABLE_GLOBAL_INT();
}

void gpBle_DataRxQueueOpenConnection(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
    Ble_RxLinkContext[connId].connId = connId;
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
}

void gpBle_DataRxQueueCloseConnection(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
    Ble_DataRxResetAuthPayloadTimers(connId);
    Ble_RxLinkContext[connId].authPayloadToUnit = GP_HCI_AUTHENTICATED_PAYLOAD_TO_DEFAULT;
    gpBle_DataRxQueueEnableDecryption(connId, false);
    Ble_RxLinkContext[connId].connId = BLE_CONN_HANDLE_INVALID;
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
}

#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
UInt16 gpBle_DataRxQueueReadAuthPayloadTo(Ble_IntConnId_t connId)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    return Ble_RxLinkContext[connId].authPayloadToUnit;
}

void gpBle_DataRxQueueWriteAuthPayloadTo(Ble_IntConnId_t connId, UInt16 authPayloadToUnit)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    Ble_RxLinkContext[connId].authPayloadToUnit = authPayloadToUnit;

    if(BLE_ENCRYPTION_IS_ENABLED(connId))
    {
        // Re-enable the authenticated payload timeout
        Ble_DataRxQueueEnableAuthPayloadTimers(connId, true);
    }
}

void gpBle_DataRxQueueEnableAuthPayloadTo(Ble_IntConnId_t connId, Bool enable)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    // Start/Stop Authenticated Payload (Nearly) Timeout
    Ble_DataRxQueueEnableAuthPayloadTimers(connId, enable);
}

#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED

void gpBle_DataRxQueueEnableDecryption(Ble_IntConnId_t connId, Bool enable)
{
    GP_ASSERT_DEV_INT(BLE_IS_INT_CONN_HANDLE_VALID(connId));

    if(enable)
    {
        BLE_ENCRYPTION_ENABLE(connId);
    }
    else
    {
        BLE_ENCRYPTION_DISABLE(connId);
    }
}

void gpBle_DataRxQueueSetFlowCtrl(UInt16 connMask)
{
    UInt8 size;

    HAL_DISABLE_GLOBAL_INT ();
    {
        /* TODO: connection-specific flow stop.
         * Would require searching through queue */
        Ble_QueueFlowConnMask = connMask;
        size = Ble_PDUQueue_size;
    }
    HAL_ENABLE_GLOBAL_INT();

    if ( BLE_IS_FLOW_STOPPED(connMask))
    {
#if defined(GP_DIVERSITY_LOG) && defined(GP_LOCAL_LOG)
        {
            UInt32 Timestamp;
            gpHal_GetTime(&Timestamp);

            GP_LOG_PRINTF("T=%lu : DRx flow OFF ",0,(unsigned long)Timestamp);
            T_FlowStop_2 = Timestamp;

        }
#endif /* GP_DIVERSITY_LOG && GP_LOCAL_LOG */
    }
    else
    {
        if (size>0)
        {
          gpSched_ScheduleEvent(0, Ble_PDUQueuePop);
        }
#if defined(GP_DIVERSITY_LOG) && defined(GP_LOCAL_LOG)
        {
            UInt32 Timestamp;
            gpHal_GetTime(&Timestamp);

            GP_LOG_PRINTF("T=%lu : DRx flow GO: DeltaT=%lu",0,(unsigned long)Timestamp, (unsigned long)(Timestamp - T_FlowStop_2));
        }
#endif /* GP_DIVERSITY_LOG && GP_LOCAL_LOG */
    }
}

UInt16 gpBle_DataRxQueueGetFlowCtrl(void)
{
    return Ble_DataRxAttributes.rxFlowCtrl;
}

#ifdef GP_DIVERSITY_DEVELOPMENT
void Ble_SetVsdAuthenticatedPayloadTimeoutEnable(Bool enable)
{
    gpBle_VsdAuthenticatedPayloadTimeoutEnable = enable;
}
void gpBle_SetVsdDataChannelRxQueueLatency(UInt32 latency)
{
    GP_LOG_PRINTF("SetVsdDataChannelRxQueueLatency %lu",0,(unsigned long)latency);

    gpBle_VsdDataChannelRxQueueLatency = latency;
}
#endif /* GP_DIVERSITY_DEVELOPMENT */

