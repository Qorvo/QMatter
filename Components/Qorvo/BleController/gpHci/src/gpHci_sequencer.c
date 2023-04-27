/*
 * Copyright (c) 2015-2016, GreenPeak Technologies
 * Copyright (c) 2017, Qorvo Inc
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

#include "gpHci.h"
#include "gpBle.h"
#include "gpLog.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpHci_defs.h"
#include "gpHci_subevents.h"
#include "gpHci_subevents_vsd.h"
#include "gpBleConfig.h"

#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleDataRx.h"
#include "gpBleLlcpProcedures.h"
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL

#ifdef GP_HCI_DIVERSITY_GPCOM_SERVER
#include "gpHci_server.h"
#endif //GP_HCI_DIVERSITY_GPCOM_SERVER



/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_HCI
#define GP_MAX_SEQUENCER_QUEUE_SIZE              (GP_BLE_DIVERSITY_HCI_BUFFER_AMOUNT_RX + GP_BLE_NR_OF_EVENT_BUFFERS)

#define GP_BLE_DIVERSITY_HCI_NUMISODLBUFFERS 1
#define GP_BLE_DIVERSITY_HCI_NUMISOULBUFFERS 3

#define GP_BLE_HCI_MAX_ISOFRAMEHEADERSIZE 12
#define GP_BLE_HCI_ISOTIMESTAMPSIZE 4
#define GP_BLE_DIVERSITY_HCI_MAXISOSDUSIZE 251

#define BLE_HCI_HEADER_SIZE 4

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER) || defined(GP_COMP_UNIT_TEST)
static UInt8 gpHci_SequencerSize = 0;
static UInt8 gpHci_SequencerCircularHead = 0;
static gpHci_SequencerElement_t gpHci_SequencerCircularBuff[GP_MAX_SEQUENCER_QUEUE_SIZE];
static UInt16 gpHci_OverFlowCount = 0;
#endif //GP_HCI_DIVERSITY_GPCOM_SERVER

/*****************************************************************************
 *                    Local Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Local Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Local Function Definitions
 *****************************************************************************/

static Bool gpHci_CheckEventMask(gpBle_EventBufferHandle_t eventHandle)
{
    gpHci_EventCode_t eventCode=0;
    Bool doQueue = false;

    gpHci_LEMetaSubEventCode_t subEventCode=0;

    gpBle_GetEventCodes(eventHandle, &eventCode, &subEventCode);

    if(gpBleConfig_IsEventMasked(eventCode))
    {
        doQueue = true;

        if(eventCode == gpHci_EventCode_LEMeta)
        {
            doQueue = gpBleConfig_IsLeMetaEventMasked(subEventCode);

            if(!doQueue)
            {
                gpHci_Result_t result = gpHci_ResultSuccess;
#if defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED) || defined(GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED) 
                gpHci_CommandParameters_t    command;
                gpHci_ConnectionHandle_t connectionHandle;
                gpBle_EventBuffer_t * pBuf = gpBle_EventHandleToBuffer(eventHandle);
#endif // defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED) || defined(GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED) || defined(GP_DIVERSITY_BLE_CIS_PERIPHERAL)

#ifdef GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED
                // Extra checks for meta events that are masked, controller relies on a negative reply in that case
                if(subEventCode == gpHci_LEMetaSubEventCodeLongTermKeyRequest)
                {
                    connectionHandle = pBuf->payload.metaEventParams.params.longTermKeyRequest.connectionHandle;

                    MEMCPY(&command.LeLongTermKeyRequestNegativeReply.connectionHandle, &connectionHandle, sizeof(gpHci_ConnectionHandle_t));
                    // Do not use execute command, but call function directly for now
                    result = gpBle_LeLongTermKeyRequestNegativeReply(&command, NULL);

                }
#endif //GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED

#ifdef GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
                if(subEventCode == gpHci_LEMetaSubEventCodeRemoteConnectionParameter)
                {
                    connectionHandle = pBuf->payload.metaEventParams.params.remoteConnectionParams.connectionHandle;

                    MEMCPY(&command.LeRemoteConnectionParamRequestNegReply.connectionHandle, &connectionHandle, sizeof(gpHci_ConnectionHandle_t));
                    command.LeRemoteConnectionParamRequestNegReply.reason = gpHci_ResultUnsupportedRemoteFeatureUnsupportedLmpFeature;
                    // Do not use execute command, but call function directly for now
                    result = gpBle_LeRemoteConnectionParamRequestNegReply(&command, NULL);
                }
#endif //GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED


                GP_ASSERT_DEV_INT(result == gpHci_ResultSuccess);
            }
        }
    }

    return doQueue;
}

static Bool gpHci_LEMetaHandler(gpHci_EventCode_t eventCode, gpHci_LEMetaEventParams_t* pBuf)
{
    Bool txSuccess = false;
    switch(pBuf->subEventCode)
    {
        case gpHci_LEMetaSubEventCodeConnectionComplete:
        {
            txSuccess = gpHci_LEConnectionCompleteEvent((gpHci_LEConnectionCompleteEventParams_t*)&pBuf->params.connectionComplete);
            break;
        }
        case gpHci_LEMetaSubEventCodeAdvertisingReport:
        {
            txSuccess = gpHci_LEAdvertisingReportEvent((gpHci_LeMetaAdvertisingReportParams_t*)&pBuf->params.advReport);
            break;
        }
#ifdef GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
        case gpHci_LEMetaSubEventCodeDataLengthChange:
        {
            txSuccess = gpHci_LEDataLengthChangeEvent((gpHci_LeMetaDataLengthChange_t*)&pBuf->params.dataLengthChange);
            break;
        }
#endif //GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
        case gpHci_LEMetaSubEventCodeConnectionUpdateComplete:
        {
            txSuccess = gpHci_LEConnectionUpdateCompleteEvent((gpHci_LEConnectionUpdateCompleteEventParams_t*) &pBuf->params.connectionUpdateComplete);
            break;
        }
        case gpHci_LEMetaSubEventCodeReadFeaturesComplete:
        {
            txSuccess = gpHci_LEReadRemoteFeaturesCompleteEvent((gpHci_LEReadRemoteFeaturesCompleteParams_t*) &pBuf->params.readRemoteFeaturesComplete);
            break;
        }
        case gpHci_LEMetaSubEventCodeLongTermKeyRequest:
        {
            txSuccess = gpHci_LELongTermKeyRequestEvent((gpHci_LELongTermKeyRequestParams_t*) &pBuf->params.longTermKeyRequest);
            break;
        }
        case gpHci_LEMetaSubEventCodeRemoteConnectionParameter:
        {
            txSuccess = gpHci_LERemoteConnectionParameterRequest((gpHci_LERemoteConnectionParamsEventParams_t*) &pBuf->params.remoteConnectionParams);
            break;
        }
#ifdef GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
        case gpHci_LEMetaSubEventCodeEnhancedConnectionComplete:
        {
            txSuccess = gpHci_LEEnhancedConnectionCompleteEvent((gpHci_LEEnhancedConnectionCompleteEventParams_t*) &pBuf->params.enhancedConnectionComplete);
            break;
        }
#endif // GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE
        case gpHci_LEMetaSubEventCodeDirectAdvertisingReport:
        {
            txSuccess = gpHci_LEDirectAdvertisingReportEvent((gpHci_LeMetaAdvertisingReportParams_t*) &pBuf->params.advReport);
            break;
        }
#ifdef GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
        case gpHci_LEMetaSubEventCodePhyUpdateComplete:
        {
            txSuccess = gpHci_LEPhyUpdateComplete((gpHci_LEPhyUpdateCompleteEventParams_t*) &pBuf->params.phyUpdateComplete);
            break;
        }
#endif //GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED


#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
        case gpHci_LEMetaSubEventCodeConnectionlessIqReport:
        {
            txSuccess = gpHci_LeConnectionlessIqReport((gpHci_LEConnectionlessIqReportEventParams_t*) &pBuf->params.connectionlessIqReport);
            break;
        }
        case gpHci_LEMetaSubEventCodeConnectionIqReport:
        {
            txSuccess = gpHci_LeConnectionIqReport((gpHci_LEConnectionIqReportEventParams_t*) &pBuf->params.connectionIqReport);
            break;
        }
        case gpHci_LEMetaSubEventCodeCteRequestFailed:
        {
            txSuccess = gpHci_LeCteRequestFailed((gpHci_LECteRequestFailedEventParams_t*) &pBuf->params.cteRequestFailed);
            break;
        }
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
        default:
            GP_LOG_SYSTEM_PRINTF("LEMetaHandler: unknown LEMeta subcode 0x%lx", 2, (long unsigned int) pBuf->subEventCode );
            break;
    }
    return txSuccess;
}

static Bool gpHci_VsdMetaHandler(gpHci_EventCode_t eventCode, gpHci_VsdMetaEventParams_t* pBuf)
{
    Bool txSuccess = false;
    if(gpBleConfig_IsLeMetaVSDEventMasked(pBuf->subEventCode))
    {
        switch(pBuf->subEventCode)
        {
            case gpHci_VsdSubEventFilterAcceptListModified:
            {
                txSuccess = gpHci_VsdMetaFilterAcceptListModifiedEvent((gpHci_VsdMetaFilterAcceptListModified_t*)&pBuf->params.filterAcceptListModified);
                break;
            }
            default:
                GP_LOG_SYSTEM_PRINTF("VsdMetaHandler: unknown VsdMeta subcode 0x%lx", 2, (long unsigned int) pBuf->subEventCode );
                break;
        }
    }

    return txSuccess;
}

static Bool gpHci_CommandCompleteHandler(gpBle_EventServer_t eventServerId, gpHci_EventCode_t eventCode, gpHci_CommandCompleteParams_t* pBuf)
{
    Bool txSuccess = false;
#ifdef GP_HCI_DIVERSITY_HOST_SERVER
    if (eventServerId == gpBle_EventServer_Host)
    {
        txSuccess = WcBleHost_gpHci_CommandCompleteHandler(eventCode, pBuf);
    }
    else
#endif // GP_HCI_DIVERSITY_HOST_SERVER
#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER) || defined(GP_COMP_UNIT_TEST)
    if (eventServerId == gpBle_EventServer_Wrapper)
    {
        txSuccess = gpHci_CommandCompleteEvent(eventCode, pBuf);
    }
    else
#endif // GP_HCI_DIVERSITY_GPCOM_SERVER
    {
        GP_ASSERT_DEV_EXT(false);
    }
    return txSuccess;
}

static Bool gpHci_SendHCiEventFrameToHost(gpBle_EventServer_t eventServerId, gpBle_EventBufferHandle_t eventHandle)
{
    Bool txSuccess = false;
    gpBle_EventBuffer_t eventBuffer;
    gpHci_EventCode_t eventCode;
    gpHci_EventCbPayload_t *pBuf;

    gpBle_CopyEventBuffer(&eventBuffer, eventHandle);
    eventCode = eventBuffer.eventCode;
    pBuf = &eventBuffer.payload;

    switch(eventCode)
    {
        case gpHci_EventCode_NumberOfCompletedPackets:
        {
            txSuccess = gpHci_NumberOfCompletedPacketsEvent(eventCode, (gpHci_NumberOfCompletedPackets_t*) &pBuf->numberOfCompletedPackets);
            break;
        }
        case gpHci_EventCode_CommandComplete:
        {
            txSuccess = gpHci_CommandCompleteHandler(eventServerId, eventCode, (gpHci_CommandCompleteParams_t*) &pBuf->commandCompleteParams);
            break;
        }
        case gpHci_EventCode_CommandStatus:
        {
            txSuccess = gpHci_CommandStatusEvent(eventCode, (gpHci_CommandStatusParams_t*) &pBuf->commandStatusParams );
            break;
        }
        case gpHci_EventCode_LEMeta:
        {
            txSuccess = gpHci_LEMetaHandler(eventCode, (gpHci_LEMetaEventParams_t*)&pBuf->metaEventParams );
            break;
        }
        case gpHci_EventCode_HardwareError:
        {
            txSuccess = gpHci_HardwareErrorEvent(eventCode, pBuf->hwErrorCode);
            break;
        }
#ifdef GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED
        case gpHci_EventCode_EncryptionChange:
        {
            txSuccess = gpHci_EncryptionChangeEvent(eventCode, (gpHci_EncryptionChangeParams_t*) &pBuf->encryptionChangeParams );
            break;
        }
        case gpHci_EventCode_EncryptionKeyRefreshComplete:
        {
            txSuccess = gpHci_EncryptionKeyRefreshCompleteEvent(eventCode, (gpHci_EncryptionKeyRefreshComplete_t*) &pBuf->keyRefreshComplete );
            break;
        }
#endif //GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED
#ifdef GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
        case gpHci_EventCode_AuthenticatedPayloadToExpired:
        {
            txSuccess = gpHci_AuthenticationPayloadTOEvent(eventCode, (gpHci_AuthenticatedPayloadToExpired_t*) &pBuf->authenticatedPayloadToExpired );
            break;
        }
#endif //GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
        case gpHci_EventCode_DataBufferOverflow:
        {
            txSuccess = gpHci_DataBufferOverflowEvent(eventCode, pBuf->linkType );
            break;
        }
        case gpHci_EventCode_ReadRemoteVersionInfoComplete:
        {
            txSuccess = gpHci_ReadRemoteVersionCompleteEvent(eventCode, (gpHci_ReadRemoteVersionInfoComplete_t*) &pBuf->readRemoteVersionInfoComplete);
            break;
        }
        case gpHci_EventCode_ConnectionComplete:
        {
            txSuccess = gpHci_ConnectionCompleteEvent(eventCode, (gpHci_ConnectionCompleteParams_t*) &pBuf->connectionCompleteParams);
            break;
        }
        case gpHci_EventCode_DisconnectionComplete:
        {
            txSuccess = gpHci_DisconnectionCompleteEvent(eventCode, (gpHci_DisconnectCompleteParams_t*) &pBuf->disconnectCompleteParams);
            break;
        }
        case gpHci_EventCode_VsdSinkRxIndication:
        {
            txSuccess = gpHci_VsdSinkRxIndication(eventCode, (gpHci_VsdSinkRxIndication_t*) &pBuf->vsdSinkRxIndication);
            break;
        }
        case gpHci_EventCode_VsdMeta:
        {
            txSuccess = gpHci_VsdMetaHandler(eventCode, (gpHci_VsdMetaEventParams_t*) &pBuf->VsdMetaEventParams);
            break;
        }
        case gpHci_EventCode_Invalid:
        case gpHci_EventCode_Nothing:
        default:
            GP_LOG_SYSTEM_PRINTF(" Unhandled eventCode 0x%lx", 2, (long unsigned int) eventCode );
            GP_ASSERT_SYSTEM(false);
            break;
    }
    return txSuccess;
}

#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER) || defined(GP_COMP_UNIT_TEST)
static void gpHci_SequencerPop(void)
{
    gpHci_SequencerElement_t *el;

    el = &gpHci_SequencerCircularBuff[gpHci_SequencerCircularHead];
    switch (el->elementType)
    {
        case SequencedEvent:
            GP_LOG_PRINTF("gpHci_SequencerPop",0);
            gpBle_HciEventConfirm(el->element.event.handle);
            break;
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case SequencedRxData:
            gpBle_HciDataRxConfirm(el->element.data.bufferIdx);
            break;
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL
        default:
            GP_ASSERT_SYSTEM(false);
            break;
    }
    gpHci_SequencerCircularHead=(UInt8)(((UInt16)gpHci_SequencerCircularHead+1)%GP_MAX_SEQUENCER_QUEUE_SIZE);
    --gpHci_SequencerSize;
}


// Sequencer implementation
static void gpHci_SequencerTxNext(void)
{
    gpHci_SequencerElement_t *el;
    Bool txSuccess = false;

    if (gpHci_SequencerSize == 0)
    {
        return;
    }

    el = &gpHci_SequencerCircularBuff[gpHci_SequencerCircularHead];
    switch (el->elementType)
    {
        case SequencedEvent:
            txSuccess = gpHci_SendHCiEventFrameToHost(gpBle_EventServer_Wrapper,
                        el->element.event.handle);
            break;
        case SequencedRxData:
            txSuccess = gpHci_SendHciDataFrameToHost(
                        (el->element.data.connHandle&0xFFF) |
                        (el->element.data.pBoundary&0x3)<<12,
                        el->element.data.dataLength,
                        el->element.data.pData);
            break;
        case SequencedRxIsoData:
        default:
            GP_ASSERT_SYSTEM(false);
    }

    GP_LOG_PRINTF("txSuccess:%x",0,txSuccess);

    if (!txSuccess)
    {
        ++gpHci_OverFlowCount;
        return;
    }

    gpHci_SequencerPop();

    if (gpHci_SequencerSize)
    {
        gpSched_ScheduleEvent(0, gpHci_SequencerTxNext);
    }
}

static gpHci_SequencerElement_t* gpHci_SequencerAllocate(void)
{
    GP_ASSERT_SYSTEM(gpHci_SequencerSize < GP_MAX_SEQUENCER_QUEUE_SIZE);
    UInt8 tail=(UInt8)(((UInt16)gpHci_SequencerCircularHead+gpHci_SequencerSize)%GP_MAX_SEQUENCER_QUEUE_SIZE);

    gpHci_SequencerCircularBuff[tail].elementType = SequencedInvalid;
    gpHci_SequencerSize++;

    if(!gpSched_ExistsEvent(gpHci_SequencerTxNext))
    {
        gpSched_ScheduleEvent(0, gpHci_SequencerTxNext);
    }

    return &gpHci_SequencerCircularBuff[tail];
}

#if !defined(GP_COMP_UNIT_TEST) // TODO: cleanup
static UInt16 gpHci_SequencerActivate( UInt16 overFlowCounter, gpCom_CommunicationId_t commId)
{
    /* Note that if we retry to send a packet we should not send logging!
     * Otherwise we will fill up the buffer again with logging and the
     * retry will keep on failing. */
    //GP_LOG_PRINTF("Reactivating HCi sequencer after %i overflows!",0,overFlowCounter);

    UInt16 retryCount = gpHci_OverFlowCount;
    gpHci_OverFlowCount = 0;
    gpSched_ScheduleEvent(0, gpHci_SequencerTxNext);
    return retryCount;
}
#endif
#endif //GP_HCI_DIVERSITY_GPCOM_SERVER || defined(GP_COMP_UNIT_TEST)

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpHci_SequencerInit(void)
{
#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER) || defined(GP_COMP_UNIT_TEST)
    gpHci_SequencerReset();
#if !defined(GP_COMP_UNIT_TEST) // TODO: cleanup
    gpHci_RegisterActivateTxCb(gpHci_SequencerActivate);
#endif
#endif //GP_HCI_DIVERSITY_GPCOM_SERVER || defined(GP_COMP_UNIT_TEST)

}

void gpHci_SequencerReset(void)
{
#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER) || defined(GP_COMP_UNIT_TEST)
    while (gpHci_SequencerSize > 0)
    {
        gpHci_SequencerPop();
    }

    gpHci_SequencerSize = 0;
    gpHci_SequencerCircularHead = 0;
    gpHci_OverFlowCount = 0;
#endif //GP_HCI_DIVERSITY_GPCOM_SERVER || defined(GP_COMP_UNIT_TEST)
}

void gpBle_cbEventIndication(gpBle_EventBufferInfo_t* pEventInfo)
{
    gpBle_EventBufferHandle_t eventHandle;
    gpBle_EventServer_t eventServerId;

    GP_LOG_PRINTF("gpBle_cbEventIndication",0);

    eventHandle = pEventInfo->eventHandle;
    eventServerId = pEventInfo->eventServer;

    if(gpHci_CheckEventMask(eventHandle))
    {
        switch (eventServerId)
        {
#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER) || defined(GP_COMP_UNIT_TEST)
        case gpBle_EventServer_Wrapper:
        {
            gpHci_SequencerElement_t * pEl;
            pEl = gpHci_SequencerAllocate();
            pEl->element.event.handle = eventHandle;
            pEl->elementType = SequencedEvent;
            break;
        }
#endif
#ifdef GP_HCI_DIVERSITY_HOST_SERVER
        case gpBle_EventServer_Host:
        {
            //Handle event without queuing
            gpHci_SendHCiEventFrameToHost(eventServerId, eventHandle);
            gpBle_HciEventConfirm(eventHandle);
            break;
        }
#endif //GP_HCI_DIVERSITY_GPCOM_SERVER || defined(GP_COMP_UNIT_TEST)
        default:
        {
            GP_ASSERT_SYSTEM(false);
        }
        }
    }
    else
    {
        GP_LOG_PRINTF("Ignored",0);
        gpBle_HciEventConfirm(eventHandle);
    }
}

void gpBle_cbDataRxIndication(gpHci_ConnectionHandle_t connHandle,
                              gpHci_PacketBoundaryFlag_t pBoundary,
                              gpBle_RxBufferHandle_t bufferId,
                              UInt16 dataLength,
                              UInt8* pData)
{
    // In our implementation, HCI ACL frames are limited to 255 bytes payload.
    // Since the Data Rx module limits received data packets to 255 bytes, there is
    // no need for the HCI Sequencer to fragment uplink data: uplink ACL data always fits in 1 HCI fragment

#if defined(GP_HCI_DIVERSITY_HOST_SERVER)
    //Handle data without queuing
    gpHci_SendHciDataFrameToHost((connHandle&0xFFF) |
                                 (pBoundary&0x3)<<12,
                                 dataLength,
                                 pData);
    gpBle_HciDataRxConfirm(bufferId);
#elif defined(GP_HCI_DIVERSITY_GPCOM_SERVER) || defined(GP_COMP_UNIT_TEST)
    gpHci_SequencerElement_t * pEl = gpHci_SequencerAllocate();
    pEl->element.data.connHandle = connHandle;
    pEl->element.data.pBoundary = pBoundary;
    pEl->element.data.bufferIdx = bufferId;
    pEl->element.data.dataLength = dataLength; // the DataRx module already has added  BLE_HCI_HEADER_SIZE
    pEl->element.data.pData = pData;
    pEl->elementType = SequencedRxData;
#else //GP_HCI_DIVERSITY_GPCOM_SERVER
#error
#endif
}


