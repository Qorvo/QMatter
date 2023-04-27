/*
 *   Copyright (c) 2015-2016, GreenPeak Technologies
 *   Copyright (c) 2017, Qorvo Inc
 *
 *   Host Controller Interface
 *   Wrapper implementation
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
 *                    Includes Definition
 *****************************************************************************/
#include "global.h"
#include "hal.h"
#include "gpUtils.h"
#include "gpLog.h"
#include "gpAssert.h"
#include "gpSched.h"
#include "gpCom.h"

#include "gpBle.h"
#include "gpModule.h"
#include "gpHci.h"
#include "gpPoolMem.h"

/*****************************************************************************
 *                    Typedef Definition
 *****************************************************************************/

/*****************************************************************************
*                    Static Functions Declaration
*****************************************************************************/

/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/

#define GP_MODULE_ID GP_MODULE_ID_HCIAPI
#define GP_COMPONENT_ID GP_COMPONENT_ID_HCI

#ifndef GP_HCI_COMM_ID
#define GP_HCI_COMM_ID GP_COM_DEFAULT_COMMUNICATION_ID
#endif

#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER)
#define DATA_REQUEST(len,buf,commId)   GP_COM_DATA_REQUEST(len,buf,commId)
#endif

/* <CodeGenerator Placeholder> AdditionalMacros */
#define UINT8_SAFE_SUB(a,b) do{     \
    if(b>a)                         \
    {                               \
        GP_LOG_SYSTEM_PRINTF("going negative line %d a %d b %d ",2, __LINE__, a, b); \
        GP_ASSERT_DEV_EXT(0);       \
    }                               \
    else                            \
    {                               \
        a-=b;                       \
    }                               \
}while(0)

// 45 bytes: ||evcode (1)|length (1)|subevcode (1)|num_reports (1)|event_type (1)|address_type (1)|address (6)|data_length (1)|data (31)|rssi (1)||
// Limited to 45 bytes, because we only allow up to 1 full report in an event.
#define GPHCI_ADVERTISEMENT_BUFSIZE_NORMAL (1 + 1 + 1 + 1 + 1 + 1 + 6 + 1 + 31 + 1)
// 45 bytes: ||evcode (1)|length (1)|subevcode (1)|num_reports (1)|event_type (1)|address_type (1)|address (6)|direct_address_type (1)|direct_address (6)|rssi (1)||
// Limited to 45 bytes, because we only allow up to 1 full report in an event.
#define GPHCI_ADVERTISEMENT_BUFSIZE_DIRECT (1 + 1 + 1 + 1 + 1 + 1 + 6 + 1 + 6 + 1)

/* </CodeGenerator Placeholder> AdditionalMacros */
/*****************************************************************************
*                    Static Data
*****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    gpHci_subevents Indications
 *****************************************************************************/

Bool gpHci_LEConnectionCompleteEvent(gpHci_LEConnectionCompleteEventParams_t* LEConnectionCompleteEventParams)
{
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (1 + 2*1 + 1 + 1 + 6*1 + 2*1 + 2*1 + 2*1 + 1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LEConnectionCompleteEventParamsPacket               (&dataBuf[1 + 1 + 1 + 0])
#define LEConnectionCompleteEventParamsPacket_status        dataBuf[1 + 1 + 1 + 0]
#define LEConnectionCompleteEventParamsPacket_connectionHandle dataBuf[1 + 1 + 1 + 0 + 1]
#define LEConnectionCompleteEventParamsPacket_role          dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1]
#define LEConnectionCompleteEventParamsPacket_peerAddressType dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1]
#define LEConnectionCompleteEventParamsPacket_peerAddress   dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1]
#define LEConnectionCompleteEventParamsPacket_connInterval  dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1]
#define LEConnectionCompleteEventParamsPacket_connLatency   dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1 + (2) * 1]
#define LEConnectionCompleteEventParamsPacket_supervisionTo dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1 + (2) * 1 + (2) * 1]
#define LEConnectionCompleteEventParamsPacket_masterClockAccuracy dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1 + (2) * 1 + (2) * 1 + (2) * 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LEConnectionCompleteEventParams->connectionHandle);
    HOST_TO_LITTLE_UINT16(&LEConnectionCompleteEventParams->connInterval);
    HOST_TO_LITTLE_UINT16(&LEConnectionCompleteEventParams->connLatency);
    HOST_TO_LITTLE_UINT16(&LEConnectionCompleteEventParams->supervisionTo);
    LEConnectionCompleteEventParamsPacket_status = LEConnectionCompleteEventParams->status;
    MEMCPY(&(LEConnectionCompleteEventParamsPacket_connectionHandle), (UInt8*)&(LEConnectionCompleteEventParams->connectionHandle), (2) * 1);
    LEConnectionCompleteEventParamsPacket_role = LEConnectionCompleteEventParams->role;
    LEConnectionCompleteEventParamsPacket_peerAddressType = LEConnectionCompleteEventParams->peerAddressType;
    MEMCPY(&(LEConnectionCompleteEventParamsPacket_peerAddress), (UInt8*)&(LEConnectionCompleteEventParams->peerAddress), (6) * 1);
    MEMCPY(&(LEConnectionCompleteEventParamsPacket_connInterval), (UInt8*)&(LEConnectionCompleteEventParams->connInterval), (2) * 1);
    MEMCPY(&(LEConnectionCompleteEventParamsPacket_connLatency), (UInt8*)&(LEConnectionCompleteEventParams->connLatency), (2) * 1);
    MEMCPY(&(LEConnectionCompleteEventParamsPacket_supervisionTo), (UInt8*)&(LEConnectionCompleteEventParams->supervisionTo), (2) * 1);
    LEConnectionCompleteEventParamsPacket_masterClockAccuracy = LEConnectionCompleteEventParams->masterClockAccuracy;
    LITTLE_TO_HOST_UINT16(&LEConnectionCompleteEventParams->connectionHandle);
    LITTLE_TO_HOST_UINT16(&LEConnectionCompleteEventParams->connInterval);
    LITTLE_TO_HOST_UINT16(&LEConnectionCompleteEventParams->connLatency);
    LITTLE_TO_HOST_UINT16(&LEConnectionCompleteEventParams->supervisionTo);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeConnectionComplete;
    lengthEvent = 19;

    // Transmit packet
    retVal = DATA_REQUEST(21, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LEConnectionCompleteEventParamsPacket
#undef LEConnectionCompleteEventParamsPacket_status
#undef LEConnectionCompleteEventParamsPacket_connectionHandle
#undef LEConnectionCompleteEventParamsPacket_role
#undef LEConnectionCompleteEventParamsPacket_peerAddressType
#undef LEConnectionCompleteEventParamsPacket_peerAddress
#undef LEConnectionCompleteEventParamsPacket_connInterval
#undef LEConnectionCompleteEventParamsPacket_connLatency
#undef LEConnectionCompleteEventParamsPacket_supervisionTo
#undef LEConnectionCompleteEventParamsPacket_masterClockAccuracy
}

Bool gpHci_LEAdvertisingReportEvent(gpHci_LeMetaAdvertisingReportParams_t* LEAdvertisingReportParams)
{
/* <CodeGenerator Placeholder> gpHci_LEAdvertisingReportEvent_AdditionalManual */
    Bool txSuccess;
    UIntLoop i;
    UInt8 dataBuf[GPHCI_ADVERTISEMENT_BUFSIZE_NORMAL];
    UInt8 *ptr;
    UInt8 *LengthField;
    UInt8 sentinel = GPHCI_ADVERTISEMENT_BUFSIZE_NORMAL;

    // Ref BT4.2 7.7.65.2
    ptr = dataBuf;
    *ptr++ = gpHci_EventCode_LEMeta;
    LengthField = ptr; //remember position of length field
    ptr++;
    *ptr++ = gpHci_LEMetaSubEventCodeAdvertisingReport;
    *ptr++ = LEAdvertisingReportParams->nrOfReports;
    sentinel-=3; // eventCode + subEvent + LEAdvertisingReportParams->nrOfReports
    for(i=0; i<LEAdvertisingReportParams->nrOfReports; i++)
    {
        UINT8_SAFE_SUB(sentinel,sizeof(LEAdvertisingReportParams->reports[i].eventType));
        *ptr++ = LEAdvertisingReportParams->reports[i].eventType;
        UINT8_SAFE_SUB(sentinel,sizeof(LEAdvertisingReportParams->reports[i].addressType));
        *ptr++ = LEAdvertisingReportParams->reports[i].addressType;
        UINT8_SAFE_SUB(sentinel,sizeof(BtDeviceAddress_t));
        MEMCPY(ptr, LEAdvertisingReportParams->reports[i].address.addr, sizeof(BtDeviceAddress_t)); ptr+=sizeof(BtDeviceAddress_t);
        *ptr++ = LEAdvertisingReportParams->reports[i].data.undirected.dataLength;
        UINT8_SAFE_SUB(sentinel,1);
        if (LEAdvertisingReportParams->reports[i].data.undirected.dataLength > 0)
        {
          UINT8_SAFE_SUB(sentinel,LEAdvertisingReportParams->reports[i].data.undirected.dataLength);
          MEMCPY(ptr, LEAdvertisingReportParams->reports[i].data.undirected.data, LEAdvertisingReportParams->reports[i].data.undirected.dataLength);
        }
        ptr += LEAdvertisingReportParams->reports[i].data.undirected.dataLength;
        UINT8_SAFE_SUB(sentinel,sizeof(LEAdvertisingReportParams->reports[i].data.undirected.rssi));
        *ptr++ = LEAdvertisingReportParams->reports[i].data.undirected.rssi;
    }

    *LengthField = ptr - LengthField - 1 /* ptr was also udpated after rssi for last entry  */;
    txSuccess = DATA_REQUEST(2 + (*LengthField), dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);

    return txSuccess;
/* </CodeGenerator Placeholder> gpHci_LEAdvertisingReportEvent_AdditionalManual */
}

Bool gpHci_LEConnectionUpdateCompleteEvent(gpHci_LEConnectionUpdateCompleteEventParams_t* LEConnectionUpdateCompleteEvent)
{
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (1 + 2*1 + 2*1 + 2*1 + 2*1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LEConnectionUpdateCompleteEventPacket               (&dataBuf[1 + 1 + 1 + 0])
#define LEConnectionUpdateCompleteEventPacket_status        dataBuf[1 + 1 + 1 + 0]
#define LEConnectionUpdateCompleteEventPacket_connectionHandle dataBuf[1 + 1 + 1 + 0 + 1]
#define LEConnectionUpdateCompleteEventPacket_connInterval  dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1]
#define LEConnectionUpdateCompleteEventPacket_connLatency   dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + (2) * 1]
#define LEConnectionUpdateCompleteEventPacket_supervisionTo dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + (2) * 1 + (2) * 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LEConnectionUpdateCompleteEvent->connectionHandle);
    HOST_TO_LITTLE_UINT16(&LEConnectionUpdateCompleteEvent->connInterval);
    HOST_TO_LITTLE_UINT16(&LEConnectionUpdateCompleteEvent->connLatency);
    HOST_TO_LITTLE_UINT16(&LEConnectionUpdateCompleteEvent->supervisionTo);
    LEConnectionUpdateCompleteEventPacket_status = LEConnectionUpdateCompleteEvent->status;
    MEMCPY(&(LEConnectionUpdateCompleteEventPacket_connectionHandle), (UInt8*)&(LEConnectionUpdateCompleteEvent->connectionHandle), (2) * 1);
    MEMCPY(&(LEConnectionUpdateCompleteEventPacket_connInterval), (UInt8*)&(LEConnectionUpdateCompleteEvent->connInterval), (2) * 1);
    MEMCPY(&(LEConnectionUpdateCompleteEventPacket_connLatency), (UInt8*)&(LEConnectionUpdateCompleteEvent->connLatency), (2) * 1);
    MEMCPY(&(LEConnectionUpdateCompleteEventPacket_supervisionTo), (UInt8*)&(LEConnectionUpdateCompleteEvent->supervisionTo), (2) * 1);
    LITTLE_TO_HOST_UINT16(&LEConnectionUpdateCompleteEvent->connectionHandle);
    LITTLE_TO_HOST_UINT16(&LEConnectionUpdateCompleteEvent->connInterval);
    LITTLE_TO_HOST_UINT16(&LEConnectionUpdateCompleteEvent->connLatency);
    LITTLE_TO_HOST_UINT16(&LEConnectionUpdateCompleteEvent->supervisionTo);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeConnectionUpdateComplete;
    lengthEvent = 10;

    // Transmit packet
    retVal = DATA_REQUEST(12, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LEConnectionUpdateCompleteEventPacket
#undef LEConnectionUpdateCompleteEventPacket_status
#undef LEConnectionUpdateCompleteEventPacket_connectionHandle
#undef LEConnectionUpdateCompleteEventPacket_connInterval
#undef LEConnectionUpdateCompleteEventPacket_connLatency
#undef LEConnectionUpdateCompleteEventPacket_supervisionTo
}

Bool gpHci_LEReadRemoteFeaturesCompleteEvent(gpHci_LEReadRemoteFeaturesCompleteParams_t* LEReadRemoteFeaturesComplete)
{
/* <CodeGenerator Placeholder> gpHci_LEReadRemoteFeaturesCompleteEvent_AdditionalManual */
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (1 + 2*1 + GP_HCI_FEATURE_SET_SIZE)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LEReadRemoteFeaturesCompletePacket              (&dataBuf[1 + 1 + 1 + 0])
#define LEReadRemoteFeaturesCompletePacket_status       dataBuf[1 + 1 + 1 + 0]
#define LEReadRemoteFeaturesCompletePacket_connectionHandle dataBuf[1 + 1 + 1 + 0 + 1]
#define LEReadRemoteFeaturesCompletePacket_features    dataBuf[1 + 1 + 1 + 0 + 1 + 2*1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LEReadRemoteFeaturesComplete->connectionHandle);
    LEReadRemoteFeaturesCompletePacket_status = LEReadRemoteFeaturesComplete->status;
    MEMCPY(&(LEReadRemoteFeaturesCompletePacket_connectionHandle), (UInt8*)&(LEReadRemoteFeaturesComplete->connectionHandle), 2*1);
    // Manual
    MEMCPY(&(LEReadRemoteFeaturesCompletePacket_features), (UInt8*)(LEReadRemoteFeaturesComplete->features), GP_HCI_FEATURE_SET_SIZE);
    LITTLE_TO_HOST_UINT16(&LEReadRemoteFeaturesComplete->connectionHandle);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeReadFeaturesComplete;
    lengthEvent = 12;

    // Transmit packet
    retVal = DATA_REQUEST(14, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LEReadRemoteFeaturesCompletePacket
#undef LEReadRemoteFeaturesCompletePacket_status
#undef LEReadRemoteFeaturesCompletePacket_connectionHandle
#undef LEReadRemoteFeaturesCompletePacket_features
/* </CodeGenerator Placeholder> gpHci_LEReadRemoteFeaturesCompleteEvent_AdditionalManual */
}

Bool gpHci_LELongTermKeyRequestEvent(gpHci_LELongTermKeyRequestParams_t* LELongTermKeyRequestEvent)
{
/* <CodeGenerator Placeholder> gpHci_LELongTermKeyRequestEvent_AdditionalManual */
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (2*1 + 8 + 2*1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LELongTermKeyRequestEventPacket                     (&dataBuf[1 + 1 + 1 + 0])
#define LELongTermKeyRequestEventPacket_connectionHandle    dataBuf[1 + 1 + 1 + 0]
#define LELongTermKeyRequestEventPacket_randomNumber        dataBuf[1 + 1 + 1 + 0 + 2*1]
#define LELongTermKeyRequestEventPacket_encryptedDiversifier dataBuf[1 + 1 + 1 + 0 + 2*1 + 8]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LELongTermKeyRequestEvent->connectionHandle);
    HOST_TO_LITTLE_UINT16(&LELongTermKeyRequestEvent->encryptedDiversifier);
    MEMCPY(&(LELongTermKeyRequestEventPacket_connectionHandle), (UInt8*)&(LELongTermKeyRequestEvent->connectionHandle), 2*1);
    // Manual
    MEMCPY(&(LELongTermKeyRequestEventPacket_randomNumber), (UInt8*)(LELongTermKeyRequestEvent->randomNumber), 8);
    MEMCPY(&(LELongTermKeyRequestEventPacket_encryptedDiversifier), (UInt8*)&(LELongTermKeyRequestEvent->encryptedDiversifier), 2*1);
    LITTLE_TO_HOST_UINT16(&LELongTermKeyRequestEvent->connectionHandle);
    LITTLE_TO_HOST_UINT16(&LELongTermKeyRequestEvent->encryptedDiversifier);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeLongTermKeyRequest;
    lengthEvent = 13;

    // Transmit packet
    retVal = DATA_REQUEST(15, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LELongTermKeyRequestEventPacket
#undef LELongTermKeyRequestEventPacket_connectionHandle
#undef LELongTermKeyRequestEventPacket_randomNumber
#undef LELongTermKeyRequestEventPacket_encryptedDiversifier
/* </CodeGenerator Placeholder> gpHci_LELongTermKeyRequestEvent_AdditionalManual */
}

Bool gpHci_LERemoteConnectionParameterRequest(gpHci_LERemoteConnectionParamsEventParams_t* LERemoteConnectionParameterRequest)
{
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (2*1 + 2*1 + 2*1 + 2*1 + 2*1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LERemoteConnectionParameterRequestPacket            (&dataBuf[1 + 1 + 1 + 0])
#define LERemoteConnectionParameterRequestPacket_connectionHandle dataBuf[1 + 1 + 1 + 0]
#define LERemoteConnectionParameterRequestPacket_connIntervalMin dataBuf[1 + 1 + 1 + 0 + (2) * 1]
#define LERemoteConnectionParameterRequestPacket_connIntervalMax dataBuf[1 + 1 + 1 + 0 + (2) * 1 + (2) * 1]
#define LERemoteConnectionParameterRequestPacket_connLatency dataBuf[1 + 1 + 1 + 0 + (2) * 1 + (2) * 1 + (2) * 1]
#define LERemoteConnectionParameterRequestPacket_supervisionTimeout dataBuf[1 + 1 + 1 + 0 + (2) * 1 + (2) * 1 + (2) * 1 + (2) * 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LERemoteConnectionParameterRequest->connectionHandle);
    HOST_TO_LITTLE_UINT16(&LERemoteConnectionParameterRequest->connIntervalMin);
    HOST_TO_LITTLE_UINT16(&LERemoteConnectionParameterRequest->connIntervalMax);
    HOST_TO_LITTLE_UINT16(&LERemoteConnectionParameterRequest->connLatency);
    HOST_TO_LITTLE_UINT16(&LERemoteConnectionParameterRequest->supervisionTimeout);
    MEMCPY(&(LERemoteConnectionParameterRequestPacket_connectionHandle), (UInt8*)&(LERemoteConnectionParameterRequest->connectionHandle), (2) * 1);
    MEMCPY(&(LERemoteConnectionParameterRequestPacket_connIntervalMin), (UInt8*)&(LERemoteConnectionParameterRequest->connIntervalMin), (2) * 1);
    MEMCPY(&(LERemoteConnectionParameterRequestPacket_connIntervalMax), (UInt8*)&(LERemoteConnectionParameterRequest->connIntervalMax), (2) * 1);
    MEMCPY(&(LERemoteConnectionParameterRequestPacket_connLatency), (UInt8*)&(LERemoteConnectionParameterRequest->connLatency), (2) * 1);
    MEMCPY(&(LERemoteConnectionParameterRequestPacket_supervisionTimeout), (UInt8*)&(LERemoteConnectionParameterRequest->supervisionTimeout), (2) * 1);
    LITTLE_TO_HOST_UINT16(&LERemoteConnectionParameterRequest->connectionHandle);
    LITTLE_TO_HOST_UINT16(&LERemoteConnectionParameterRequest->connIntervalMin);
    LITTLE_TO_HOST_UINT16(&LERemoteConnectionParameterRequest->connIntervalMax);
    LITTLE_TO_HOST_UINT16(&LERemoteConnectionParameterRequest->connLatency);
    LITTLE_TO_HOST_UINT16(&LERemoteConnectionParameterRequest->supervisionTimeout);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeRemoteConnectionParameter;
    lengthEvent = 11;

    // Transmit packet
    retVal = DATA_REQUEST(13, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LERemoteConnectionParameterRequestPacket
#undef LERemoteConnectionParameterRequestPacket_connectionHandle
#undef LERemoteConnectionParameterRequestPacket_connIntervalMin
#undef LERemoteConnectionParameterRequestPacket_connIntervalMax
#undef LERemoteConnectionParameterRequestPacket_connLatency
#undef LERemoteConnectionParameterRequestPacket_supervisionTimeout
}

Bool gpHci_LEDataLengthChangeEvent(gpHci_LeMetaDataLengthChange_t* LeMetaDataLengthChangeEvent)
{
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (2*1 + 2*1 + 2*1 + 2*1 + 2*1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LeMetaDataLengthChangeEventPacket                   (&dataBuf[1 + 1 + 1 + 0])
#define LeMetaDataLengthChangeEventPacket_connectionHandle  dataBuf[1 + 1 + 1 + 0]
#define LeMetaDataLengthChangeEventPacket_MaxTxOctets       dataBuf[1 + 1 + 1 + 0 + (2) * 1]
#define LeMetaDataLengthChangeEventPacket_MaxTxTime         dataBuf[1 + 1 + 1 + 0 + (2) * 1 + (2) * 1]
#define LeMetaDataLengthChangeEventPacket_MaxRxOctets       dataBuf[1 + 1 + 1 + 0 + (2) * 1 + (2) * 1 + (2) * 1]
#define LeMetaDataLengthChangeEventPacket_MaxRxTime         dataBuf[1 + 1 + 1 + 0 + (2) * 1 + (2) * 1 + (2) * 1 + (2) * 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LeMetaDataLengthChangeEvent->connectionHandle);
    HOST_TO_LITTLE_UINT16(&LeMetaDataLengthChangeEvent->MaxTxOctets);
    HOST_TO_LITTLE_UINT16(&LeMetaDataLengthChangeEvent->MaxTxTime);
    HOST_TO_LITTLE_UINT16(&LeMetaDataLengthChangeEvent->MaxRxOctets);
    HOST_TO_LITTLE_UINT16(&LeMetaDataLengthChangeEvent->MaxRxTime);
    MEMCPY(&(LeMetaDataLengthChangeEventPacket_connectionHandle), (UInt8*)&(LeMetaDataLengthChangeEvent->connectionHandle), (2) * 1);
    MEMCPY(&(LeMetaDataLengthChangeEventPacket_MaxTxOctets), (UInt8*)&(LeMetaDataLengthChangeEvent->MaxTxOctets), (2) * 1);
    MEMCPY(&(LeMetaDataLengthChangeEventPacket_MaxTxTime), (UInt8*)&(LeMetaDataLengthChangeEvent->MaxTxTime), (2) * 1);
    MEMCPY(&(LeMetaDataLengthChangeEventPacket_MaxRxOctets), (UInt8*)&(LeMetaDataLengthChangeEvent->MaxRxOctets), (2) * 1);
    MEMCPY(&(LeMetaDataLengthChangeEventPacket_MaxRxTime), (UInt8*)&(LeMetaDataLengthChangeEvent->MaxRxTime), (2) * 1);
    LITTLE_TO_HOST_UINT16(&LeMetaDataLengthChangeEvent->connectionHandle);
    LITTLE_TO_HOST_UINT16(&LeMetaDataLengthChangeEvent->MaxTxOctets);
    LITTLE_TO_HOST_UINT16(&LeMetaDataLengthChangeEvent->MaxTxTime);
    LITTLE_TO_HOST_UINT16(&LeMetaDataLengthChangeEvent->MaxRxOctets);
    LITTLE_TO_HOST_UINT16(&LeMetaDataLengthChangeEvent->MaxRxTime);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeDataLengthChange;
    lengthEvent = 11;

    // Transmit packet
    retVal = DATA_REQUEST(13, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LeMetaDataLengthChangeEventPacket
#undef LeMetaDataLengthChangeEventPacket_connectionHandle
#undef LeMetaDataLengthChangeEventPacket_MaxTxOctets
#undef LeMetaDataLengthChangeEventPacket_MaxTxTime
#undef LeMetaDataLengthChangeEventPacket_MaxRxOctets
#undef LeMetaDataLengthChangeEventPacket_MaxRxTime
}

#if defined(GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE)
Bool gpHci_LEEnhancedConnectionCompleteEvent(gpHci_LEEnhancedConnectionCompleteEventParams_t* LEEnhancedConnectionCompleteEventParams)
{
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (1 + 2*1 + 1 + 1 + 6*1 + 6*1 + 6*1 + 2*1 + 2*1 + 2*1 + 1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LEEnhancedConnectionCompleteEventParamsPacket       (&dataBuf[1 + 1 + 1 + 0])
#define LEEnhancedConnectionCompleteEventParamsPacket_status dataBuf[1 + 1 + 1 + 0]
#define LEEnhancedConnectionCompleteEventParamsPacket_connectionHandle dataBuf[1 + 1 + 1 + 0 + 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_role  dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_peerAddressType dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_peerAddress dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_localPrivateAddress dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_peerPrivateAddress dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1 + (6) * 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_connInterval dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1 + (6) * 1 + (6) * 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_connLatency dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1 + (6) * 1 + (6) * 1 + (2) * 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_supervisionTo dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1 + (6) * 1 + (6) * 1 + (2) * 1 + (2) * 1]
#define LEEnhancedConnectionCompleteEventParamsPacket_masterClockAccuracy dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1 + 1 + (6) * 1 + (6) * 1 + (6) * 1 + (2) * 1 + (2) * 1 + (2) * 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LEEnhancedConnectionCompleteEventParams->connectionHandle);
    HOST_TO_LITTLE_UINT16(&LEEnhancedConnectionCompleteEventParams->connInterval);
    HOST_TO_LITTLE_UINT16(&LEEnhancedConnectionCompleteEventParams->connLatency);
    HOST_TO_LITTLE_UINT16(&LEEnhancedConnectionCompleteEventParams->supervisionTo);
    LEEnhancedConnectionCompleteEventParamsPacket_status = LEEnhancedConnectionCompleteEventParams->status;
    MEMCPY(&(LEEnhancedConnectionCompleteEventParamsPacket_connectionHandle), (UInt8*)&(LEEnhancedConnectionCompleteEventParams->connectionHandle), (2) * 1);
    LEEnhancedConnectionCompleteEventParamsPacket_role = LEEnhancedConnectionCompleteEventParams->role;
    LEEnhancedConnectionCompleteEventParamsPacket_peerAddressType = LEEnhancedConnectionCompleteEventParams->peerAddressType;
    MEMCPY(&(LEEnhancedConnectionCompleteEventParamsPacket_peerAddress), (UInt8*)&(LEEnhancedConnectionCompleteEventParams->peerAddress), (6) * 1);
    MEMCPY(&(LEEnhancedConnectionCompleteEventParamsPacket_localPrivateAddress), (UInt8*)&(LEEnhancedConnectionCompleteEventParams->localPrivateAddress), (6) * 1);
    MEMCPY(&(LEEnhancedConnectionCompleteEventParamsPacket_peerPrivateAddress), (UInt8*)&(LEEnhancedConnectionCompleteEventParams->peerPrivateAddress), (6) * 1);
    MEMCPY(&(LEEnhancedConnectionCompleteEventParamsPacket_connInterval), (UInt8*)&(LEEnhancedConnectionCompleteEventParams->connInterval), (2) * 1);
    MEMCPY(&(LEEnhancedConnectionCompleteEventParamsPacket_connLatency), (UInt8*)&(LEEnhancedConnectionCompleteEventParams->connLatency), (2) * 1);
    MEMCPY(&(LEEnhancedConnectionCompleteEventParamsPacket_supervisionTo), (UInt8*)&(LEEnhancedConnectionCompleteEventParams->supervisionTo), (2) * 1);
    LEEnhancedConnectionCompleteEventParamsPacket_masterClockAccuracy = LEEnhancedConnectionCompleteEventParams->masterClockAccuracy;
    LITTLE_TO_HOST_UINT16(&LEEnhancedConnectionCompleteEventParams->connectionHandle);
    LITTLE_TO_HOST_UINT16(&LEEnhancedConnectionCompleteEventParams->connInterval);
    LITTLE_TO_HOST_UINT16(&LEEnhancedConnectionCompleteEventParams->connLatency);
    LITTLE_TO_HOST_UINT16(&LEEnhancedConnectionCompleteEventParams->supervisionTo);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeEnhancedConnectionComplete;
    lengthEvent = 31;

    // Transmit packet
    retVal = DATA_REQUEST(33, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LEEnhancedConnectionCompleteEventParamsPacket
#undef LEEnhancedConnectionCompleteEventParamsPacket_status
#undef LEEnhancedConnectionCompleteEventParamsPacket_connectionHandle
#undef LEEnhancedConnectionCompleteEventParamsPacket_role
#undef LEEnhancedConnectionCompleteEventParamsPacket_peerAddressType
#undef LEEnhancedConnectionCompleteEventParamsPacket_peerAddress
#undef LEEnhancedConnectionCompleteEventParamsPacket_localPrivateAddress
#undef LEEnhancedConnectionCompleteEventParamsPacket_peerPrivateAddress
#undef LEEnhancedConnectionCompleteEventParamsPacket_connInterval
#undef LEEnhancedConnectionCompleteEventParamsPacket_connLatency
#undef LEEnhancedConnectionCompleteEventParamsPacket_supervisionTo
#undef LEEnhancedConnectionCompleteEventParamsPacket_masterClockAccuracy
}
#endif /* defined(GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE) */

Bool gpHci_LEDirectAdvertisingReportEvent(gpHci_LeMetaAdvertisingReportParams_t* LEDirectAdvertisingReportParams)
{
/* <CodeGenerator Placeholder> gpHci_LEDirectAdvertisingReportEvent_AdditionalManual */
    Bool retVal;
    UIntLoop i;
    UInt8 dataBuf[GPHCI_ADVERTISEMENT_BUFSIZE_DIRECT];
    UInt8 *ptr;
    UInt8 *LengthField;
    UInt8 sentinel = GPHCI_ADVERTISEMENT_BUFSIZE_DIRECT;
    // Ref BT4.2 7.7.65.11
    ptr = dataBuf;
    *ptr++ = gpHci_EventCode_LEMeta;
    LengthField = ptr; //remember position of length field
    ptr++;
    *ptr++ = gpHci_LEMetaSubEventCodeDirectAdvertisingReport;
    *ptr++ = LEDirectAdvertisingReportParams->nrOfReports;
    sentinel-=3; // eventCode + subEvent + LEAdvertisingReportParams->nrOfReports
    for(i=0; i<LEDirectAdvertisingReportParams->nrOfReports; i++)
    {
        UINT8_SAFE_SUB(sentinel,sizeof(LEDirectAdvertisingReportParams->reports[i].eventType));
        *ptr++ = LEDirectAdvertisingReportParams->reports[i].eventType;
        UINT8_SAFE_SUB(sentinel,sizeof(LEDirectAdvertisingReportParams->reports[i].addressType));
        *ptr++ = LEDirectAdvertisingReportParams->reports[i].addressType;
        UINT8_SAFE_SUB(sentinel,sizeof(BtDeviceAddress_t));
        MEMCPY(ptr, LEDirectAdvertisingReportParams->reports[i].address.addr, sizeof(BtDeviceAddress_t)); ptr+=sizeof(BtDeviceAddress_t);
        UINT8_SAFE_SUB(sentinel,sizeof(LEDirectAdvertisingReportParams->reports[i].data.directed.directAddressType));
        *ptr++ = LEDirectAdvertisingReportParams->reports[i].data.directed.directAddressType;
        UINT8_SAFE_SUB(sentinel,sizeof(BtDeviceAddress_t));
        MEMCPY(ptr, LEDirectAdvertisingReportParams->reports[i].data.directed.directAddress.addr, sizeof(BtDeviceAddress_t));
        ptr += sizeof(BtDeviceAddress_t);
        UINT8_SAFE_SUB(sentinel,sizeof(LEDirectAdvertisingReportParams->reports[i].data.directed.rssi));
        *ptr++ = LEDirectAdvertisingReportParams->reports[i].data.directed.rssi;
    }

    *LengthField = ptr - LengthField - 1 /* ptr was also udpated after rssi for last entry  */;
    retVal = DATA_REQUEST(2 + (*LengthField), dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);

    return retVal;
/* </CodeGenerator Placeholder> gpHci_LEDirectAdvertisingReportEvent_AdditionalManual */
}

#if defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED)
Bool gpHci_LEPhyUpdateComplete(gpHci_LEPhyUpdateCompleteEventParams_t* LEPhyUpdateComplete)
{
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (1 + 2*1 + 1 + 1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LEPhyUpdateCompletePacket                           (&dataBuf[1 + 1 + 1 + 0])
#define LEPhyUpdateCompletePacket_status                    dataBuf[1 + 1 + 1 + 0]
#define LEPhyUpdateCompletePacket_connectionHandle          dataBuf[1 + 1 + 1 + 0 + 1]
#define LEPhyUpdateCompletePacket_txPhy                     dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1]
#define LEPhyUpdateCompletePacket_rxPhy                     dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LEPhyUpdateComplete->connectionHandle);
    LEPhyUpdateCompletePacket_status = LEPhyUpdateComplete->status;
    MEMCPY(&(LEPhyUpdateCompletePacket_connectionHandle), (UInt8*)&(LEPhyUpdateComplete->connectionHandle), (2) * 1);
    LEPhyUpdateCompletePacket_txPhy = LEPhyUpdateComplete->txPhy;
    LEPhyUpdateCompletePacket_rxPhy = LEPhyUpdateComplete->rxPhy;
    LITTLE_TO_HOST_UINT16(&LEPhyUpdateComplete->connectionHandle);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodePhyUpdateComplete;
    lengthEvent = 6;

    // Transmit packet
    retVal = DATA_REQUEST(8, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LEPhyUpdateCompletePacket
#undef LEPhyUpdateCompletePacket_status
#undef LEPhyUpdateCompletePacket_connectionHandle
#undef LEPhyUpdateCompletePacket_txPhy
#undef LEPhyUpdateCompletePacket_rxPhy
}
#endif /* defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED) */









#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
Bool gpHci_LeConnectionlessIqReport(gpHci_LEConnectionlessIqReportEventParams_t* LeConnectionlessIqReport)
{
/* <CodeGenerator Placeholder> gpHci_LeConnectionlessIqReport_AdditionalManual */
    Bool retVal;
    UInt8 dataBuf[1 + 1 /* len */+ 1 /* subevtcode */ + 1 /* not needed */ + (2*1 + 1 + 2 + 1 + 1 + 1 + 1 + 2*1 + 1 + 90*(1 + 1))];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LeConnectionlessIqReportPacket                      (&dataBuf[1 + 1 + 1 + 0])
#define LeConnectionlessIqReportPacket_syncHandle           dataBuf[1 + 1 + 1 + 0]
#define LeConnectionlessIqReportPacket_channelIndex         dataBuf[1 + 1 + 1 + 0 + 2*1]
#define LeConnectionlessIqReportPacket_rssi                 dataBuf[1 + 1 + 1 + 0 + 2*1 + 1]
#define LeConnectionlessIqReportPacket_rssi_antenna_id      dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 2]
#define LeConnectionlessIqReportPacket_cteType              dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 2 + 1]
#define LeConnectionlessIqReportPacket_slotDurations        dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 2 + 1 + 1]
#define LeConnectionlessIqReportPacket_packetStatus         dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 2 + 1 + 1 + 1]
#define LeConnectionlessIqReportPacket_paEventCounter       dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 2 + 1 + 1 + 1 + 1]
#define LeConnectionlessIqReportPacket_sampleCount          dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 2 + 1 + 1 + 1 + 1 + 2*1]
#define LeConnectionlessIqReportPacket_iqSamples            dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 2 + 1 + 1 + 1 + 1 + 2*1 + 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LeConnectionlessIqReport->syncHandle);
    HOST_TO_LITTLE_UINT16(&LeConnectionlessIqReport->paEventCounter);
    MEMCPY(&(LeConnectionlessIqReportPacket_syncHandle), (UInt8*)&(LeConnectionlessIqReport->syncHandle), 2*1);
    LeConnectionlessIqReportPacket_channelIndex = LeConnectionlessIqReport->channelIndex;
    MEMCPY(&LeConnectionlessIqReportPacket_rssi, (UInt8*)&(LeConnectionlessIqReport->rssi), 2*1);
    LeConnectionlessIqReportPacket_rssi_antenna_id = LeConnectionlessIqReport->rssi_antenna_id;
    LeConnectionlessIqReportPacket_cteType = LeConnectionlessIqReport->cteType;
    LeConnectionlessIqReportPacket_slotDurations = LeConnectionlessIqReport->slotDurations;
    LeConnectionlessIqReportPacket_packetStatus = LeConnectionlessIqReport->packetStatus;
    MEMCPY(&(LeConnectionlessIqReportPacket_paEventCounter), (UInt8*)&(LeConnectionlessIqReport->paEventCounter), 2*1);
    LeConnectionlessIqReportPacket_sampleCount = LeConnectionlessIqReport->sampleCount;
    MEMCPY(&LeConnectionlessIqReportPacket_iqSamples, (UInt8*)&(LeConnectionlessIqReport->iqSamples), 2*LeConnectionlessIqReport->sampleCount);
    LITTLE_TO_HOST_UINT16(&LeConnectionlessIqReport->syncHandle);
    LITTLE_TO_HOST_UINT16(&LeConnectionlessIqReport->paEventCounter);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeConnectionlessIqReport;
    lengthEvent = 13 + 2*LeConnectionlessIqReport->sampleCount;

    // Transmit packet
    retVal = DATA_REQUEST(lengthEvent + 2, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LeConnectionlessIqReportPacket
#undef LeConnectionlessIqReportPacket_syncHandle
#undef LeConnectionlessIqReportPacket_channelIndex
#undef LeConnectionlessIqReportPacket_rssi
#undef LeConnectionlessIqReportPacket_rssi_antenna_id
#undef LeConnectionlessIqReportPacket_cteType
#undef LeConnectionlessIqReportPacket_slotDurations
#undef LeConnectionlessIqReportPacket_packetStatus
#undef LeConnectionlessIqReportPacket_paEventCounter
#undef LeConnectionlessIqReportPacket_sampleCount
#undef LeConnectionlessIqReportPacket_iqSamples
/* </CodeGenerator Placeholder> gpHci_LeConnectionlessIqReport_AdditionalManual */
}
#endif /* defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED) */

#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
Bool gpHci_LeConnectionIqReport(gpHci_LEConnectionIqReportEventParams_t* LeConnectionIqReport)
{
/* <CodeGenerator Placeholder> gpHci_LeConnectionIqReport_AdditionalManual */
    Bool retVal;
    // we support 2 microsec slots for AoA/AoD: max 90 = 2 * (8+37) I/Q samples
    UInt8 dataBuf[1 + 1 /* len */ + 1 /* subevtcode */ + (2*1 + 1 + 1 + 2 + 1 + 1 + 1 + 1 + 2 + 1 + 90)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LeConnectionIqReportPacket                          (&dataBuf[1 + 1 + 1 + 0])
#define LeConnectionIqReportPacket_connectionHandle         dataBuf[1 + 1 + 1 + 0]
#define LeConnectionIqReportPacket_rxPhy                    dataBuf[1 + 1 + 1 + 0 + 2*1]
#define LeConnectionIqReportPacket_dataChannelIndex         dataBuf[1 + 1 + 1 + 0 + 2*1 + 1]
#define LeConnectionIqReportPacket_rssi                     dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 1]
#define LeConnectionIqReportPacket_rssi_antenna_id          dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 1 + 2]
#define LeConnectionIqReportPacket_cteType                  dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 1 + 2 + 1]
#define LeConnectionIqReportPacket_slotDurations            dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 1 + 2 + 1 + 1]
#define LeConnectionIqReportPacket_packetStatus             dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 1 + 2 + 1 + 1 + 1]
#define LeConnectionIqReportPacket_connEventCounter         dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 1 + 2 + 1 + 1 + 1 + 1]
#define LeConnectionIqReportPacket_sampleCount              dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 1 + 2 + 1 + 1 + 1 + 1 + 2]
#define LeConnectionIqReportPacket_iqSamples                dataBuf[1 + 1 + 1 + 0 + 2*1 + 1 + 1 + 2 + 1 + 1 + 1 + 1 + 2 + 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LeConnectionIqReport->connectionHandle);
    MEMCPY(&(LeConnectionIqReportPacket_connectionHandle), (UInt8*)&(LeConnectionIqReport->connectionHandle), 2*1);
    LeConnectionIqReportPacket_rxPhy = LeConnectionIqReport->rxPhy;
    LeConnectionIqReportPacket_dataChannelIndex = LeConnectionIqReport->dataChannelIndex;
    MEMCPY(&LeConnectionIqReportPacket_rssi, (UInt8*)&(LeConnectionIqReport->rssi), 2*1);
    LeConnectionIqReportPacket_rssi_antenna_id = LeConnectionIqReport->rssi_antenna_id;
    LeConnectionIqReportPacket_cteType = LeConnectionIqReport->cteType;
    LeConnectionIqReportPacket_slotDurations = LeConnectionIqReport->slotDurations;
    LeConnectionIqReportPacket_packetStatus = LeConnectionIqReport->packetStatus;
    MEMCPY(&LeConnectionIqReportPacket_connEventCounter, (UInt8*)&(LeConnectionIqReport->connEventCounter), 2*1);
    LeConnectionIqReportPacket_sampleCount = LeConnectionIqReport->sampleCount;
    MEMCPY(&LeConnectionIqReportPacket_iqSamples, (UInt8*)&(LeConnectionIqReport->iqSamples), 2*LeConnectionIqReport->sampleCount);
    LITTLE_TO_HOST_UINT16(&LeConnectionIqReport->connectionHandle);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeConnectionIqReport;
    lengthEvent = 2*LeConnectionIqReport->sampleCount + 16 - 2;

    // Transmit packet
    retVal = DATA_REQUEST(lengthEvent + 2, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LeConnectionIqReportPacket
#undef LeConnectionIqReportPacket_connectionHandle
#undef LeConnectionIqReportPacket_rxPhy
#undef LeConnectionIqReportPacket_dataChannelIndex
#undef LeConnectionIqReportPacket_rssi
#undef LeConnectionIqReportPacket_rssi_antenna_id
#undef LeConnectionIqReportPacket_cteType
#undef LeConnectionIqReportPacket_slotDurations
#undef LeConnectionIqReportPacket_packetStatus
#undef LeConnectionIqReportPacket_sampleCount
#undef LeConnectionIqReportPacket_iqSamples
/* </CodeGenerator Placeholder> gpHci_LeConnectionIqReport_AdditionalManual */
}
#endif /* defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED) */

#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
Bool gpHci_LeCteRequestFailed(gpHci_LECteRequestFailedEventParams_t* LeCteRequestFailed)
{
/* <CodeGenerator Placeholder> gpHci_LeCteRequestFailed_AdditionalManual */
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (1 + 2*1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define LeCteRequestFailedPacket                            (&dataBuf[1 + 1 + 1 + 0])
#define LeCteRequestFailedPacket_status                     dataBuf[1 + 1 + 1 + 0]
#define LeCteRequestFailedPacket_connectionHandle           dataBuf[1 + 1 + 1 + 0 + 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&LeCteRequestFailed->connectionHandle);
    LeCteRequestFailedPacket_status = LeCteRequestFailed->status;
    MEMCPY(&(LeCteRequestFailedPacket_connectionHandle), (UInt8*)&(LeCteRequestFailed->connectionHandle), 2*1);
    LITTLE_TO_HOST_UINT16(&LeCteRequestFailed->connectionHandle);

    // Serialize header
    eventCode = gpHci_EventCode_LEMeta;
    subEventCode = gpHci_LEMetaSubEventCodeCteRequestFailed;
    lengthEvent = 4;

    // Transmit packet
    retVal = DATA_REQUEST(6, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef LeCteRequestFailedPacket
#undef LeCteRequestFailedPacket_status
#undef LeCteRequestFailedPacket_connectionHandle
/* </CodeGenerator Placeholder> gpHci_LeCteRequestFailed_AdditionalManual */
}
#endif /* defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED) */






