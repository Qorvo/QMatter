/*
 *   Copyright (c) 2018, Qorvo Inc
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

/*****************************************************************************
*                    Static Data
*****************************************************************************/

/* <CodeGenerator Placeholder> AdditionalStaticData */
extern gpCom_CommunicationId_t gpHci_CommId;
/* </CodeGenerator Placeholder> AdditionalStaticData */
/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/


/*****************************************************************************
 *                    gpHci_subevents_vsd Indications
 *****************************************************************************/

Bool gpHci_VsdForwardEventProcessedMessages(gpHci_VsdMetaEventProcessedParams_t* eventProcessedParams)
{
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (1 + 2*1 + 4*1 + 4*1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define eventProcessedParamsPacket                          (&dataBuf[1 + 1 + 1 + 0])
#define eventProcessedParamsPacket_connId                   dataBuf[1 + 1 + 1 + 0]
#define eventProcessedParamsPacket_eventCounter             dataBuf[1 + 1 + 1 + 0 + 1]
#define eventProcessedParamsPacket_tsLastValidPacketReceived dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1]
#define eventProcessedParamsPacket_tsLastPacketReceived     dataBuf[1 + 1 + 1 + 0 + 1 + (2) * 1 + (4) * 1]

    // Serialize payload
    HOST_TO_LITTLE_UINT16(&eventProcessedParams->eventCounter);
    HOST_TO_LITTLE_UINT32(&eventProcessedParams->tsLastValidPacketReceived);
    HOST_TO_LITTLE_UINT32(&eventProcessedParams->tsLastPacketReceived);
    eventProcessedParamsPacket_connId = eventProcessedParams->connId;
    MEMCPY(&(eventProcessedParamsPacket_eventCounter), (UInt8*)&(eventProcessedParams->eventCounter), (2) * 1);
    MEMCPY(&(eventProcessedParamsPacket_tsLastValidPacketReceived), (UInt8*)&(eventProcessedParams->tsLastValidPacketReceived), (4) * 1);
    MEMCPY(&(eventProcessedParamsPacket_tsLastPacketReceived), (UInt8*)&(eventProcessedParams->tsLastPacketReceived), (4) * 1);
    LITTLE_TO_HOST_UINT16(&eventProcessedParams->eventCounter);
    LITTLE_TO_HOST_UINT32(&eventProcessedParams->tsLastValidPacketReceived);
    LITTLE_TO_HOST_UINT32(&eventProcessedParams->tsLastPacketReceived);

    // Serialize header
    eventCode = gpHci_EventCode_VsdMeta;
    subEventCode = gpHci_VsdSubEventForwardEventProcessedMessages;
    lengthEvent = 12;

    // Transmit packet
    retVal = DATA_REQUEST(14, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef eventProcessedParamsPacket
#undef eventProcessedParamsPacket_connId
#undef eventProcessedParamsPacket_eventCounter
#undef eventProcessedParamsPacket_tsLastValidPacketReceived
#undef eventProcessedParamsPacket_tsLastPacketReceived
}

Bool gpHci_VsdMetaFilterAcceptListModifiedEvent(gpHci_VsdMetaFilterAcceptListModified_t* filterAcceptListModified)
{
    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1 + (6*1 + 1)];

// Defines for offset of members in packet
#define eventCode                  dataBuf[0]
#define lengthEvent                dataBuf[1]
#define subEventCode               dataBuf[2]
#define filterAcceptListModifiedPacket                      (&dataBuf[1 + 1 + 1 + 0])
#define filterAcceptListModifiedPacket_address              dataBuf[1 + 1 + 1 + 0]
#define filterAcceptListModifiedPacket_isAdded              dataBuf[1 + 1 + 1 + 0 + (6) * 1]

    // Serialize payload
    MEMCPY(&(filterAcceptListModifiedPacket_address), (UInt8*)&(filterAcceptListModified->address), (6) * 1);
    filterAcceptListModifiedPacket_isAdded = filterAcceptListModified->isAdded;

    // Serialize header
    eventCode = gpHci_EventCode_VsdMeta;
    subEventCode = gpHci_VsdSubEventFilterAcceptListModified;
    lengthEvent = 8;

    // Transmit packet
    retVal = DATA_REQUEST(10, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_HW_4);
    return retVal;

#undef filterAcceptListModifiedPacket
#undef filterAcceptListModifiedPacket_address
#undef filterAcceptListModifiedPacket_isAdded
}



