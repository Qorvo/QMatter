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
#include "gpHci_Includes.h"
#include "gpBle.h"
#include "gpModule.h"
#include "gpHci_clientServerCmdId.h"
#include "gpHci.h"
#include "gpHci_server.h"
/* <CodeGenerator Placeholder> AdditionalIncludes */
#include "gpBleAddressResolver.h"
#include "gpBleConfig.h"
#if defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleAdvertiser.h"
#endif //GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_PERIPHERAL
#if defined(GP_DIVERSITY_BLE_OBSERVER) 
#include "gpBleScanner.h"
#endif //GP_DIVERSITY_BLE_OBSERVER || GP_DIVERSITY_BLE_CENTRAL
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleInitiator.h"
#include "gpBleDataCommon.h"
#include "gpBleDataRx.h"
#include "gpBleLlcp.h"
#include "gpBleLlcpProcedures.h"
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL
#include "gpBleSecurityCoprocessor.h"
#ifdef GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
#include "gpBleTestMode.h"
#endif //GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
#ifdef GP_COMP_BLEDIRECTIONFINDING
#include "gpBleDirectionFinding.h"
#include "gpBleLlcpProcedures_ConstantToneExtension.h"
#endif //GP_COMP_BLEDIRECTIONFINDING
#include "gpPoolMem.h"
#include "gpHci_defs.h"
#ifdef GP_COMP_BLERESPRADDR
#include "gpBleResPrAddr.h"
#endif //GP_COMP_BLERESPRADDR
#include "gpBleComps.h"
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Typedef Definition
 *****************************************************************************/

/*****************************************************************************
*                    Static Functions Declaration
*****************************************************************************/
void gpHci_HandleCommandServer(UInt16 length, UInt8* pRawPayload, gpCom_CommunicationId_t communicationId);
static void Hci_HandleConnectionClose(gpCom_CommunicationId_t communicationId);

/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_HCI
#define GP_MODULE_ID GP_COMPONENT_ID

#ifndef GP_HCI_COMM_ID
#define GP_HCI_COMM_ID GP_COM_DEFAULT_COMMUNICATION_ID
#endif

#if defined(GP_HCI_DIVERSITY_GPCOM_SERVER)
#define REGISTER_MODULE(handle)        GP_COM_REGISTER_MODULE(handle)
#define DATA_REQUEST(len,buf,commId)   GP_COM_DATA_REQUEST(len,buf,commId)
#define REGISTER_ACTIVATE_TX_CB(cb)    GP_COM_REGISTER_ACTIVATE_TX_CB(cb)
#endif

/*****************************************************************************
*                    Static Data
*****************************************************************************/
gpCom_CommunicationId_t gpHci_CommId;

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

static void Hci_HandleConnectionClose(gpCom_CommunicationId_t communicationId)
{
    if (communicationId == gpHci_CommId)
    {
        gpBleComps_ResetLinkLayer();
        gpHci_CommId = GP_HCI_COMM_ID;
    }
}

void gpHci_HandleCommandServer(UInt16 length, UInt8* pRawPayload, gpCom_CommunicationId_t communicationId)
{
#define pPayload (&pRawPayload[0])

    if(pPayload == NULL)
    {
        Hci_HandleConnectionClose(communicationId);
        return;
    }

    if (gpHci_CommId != (communicationId & ~(GP_COM_COMM_ID_BLE)))
    {
        /* save commId to identify HandleClose on peer */
        gpHci_CommId = (communicationId & ~(GP_COM_COMM_ID_BLE));
    }

    switch(communicationId & GP_COM_COMM_ID_BLE)
    {
        case GP_COM_COMM_ID_BLE_COMMAND:
        {
            gpHci_CommandOpCode_t opCode;
            UInt8 totalLength;
            UInt8 *pData;

            MEMCPY(&opCode, pPayload, sizeof(gpHci_CommandOpCode_t));
            totalLength = pPayload[2];
            pData = &pPayload[3];

            if(gpHci_commandsEnabled() == false)
            {
                //Try to store command
                if(!gpHci_StoreCrossOverCommand(opCode, totalLength, pData))
                {
                    gpHci_HardwareErrorEvent(gpHci_EventCode_HardwareError, gpHci_BufferOverflow);
                }
            }
            else
            {
                gpHci_processCommand(opCode, totalLength, pData);
            }
            break;
        }
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case GP_COM_COMM_ID_BLE_DATA:
        {
            UInt16 handle;
            UInt16 totalLength;
            UInt8* pData;
            MEMCPY(&handle, pPayload, sizeof(UInt16));
            MEMCPY(&totalLength, &pPayload[2], sizeof(UInt16));
            pData = &pPayload[4];

            gpHci_processData(handle, totalLength, pData);
            break;
        }
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL
        case GP_COM_COMM_ID_BLE_EVENT: // fall through
        default:
        {
            //??
            break;
        }
    }

#undef pPayload

}
/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpHci_InitServer(void)
{
     gpHci_CommId = GP_HCI_COMM_ID;
     REGISTER_MODULE(gpHci_HandleCommandServer);
}

void gpHci_RegisterActivateTxCb(gpCom_cbActivateTx_t cb)
{
    REGISTER_ACTIVATE_TX_CB(cb);
}

/*****************************************************************************
 *                    gpHci Indications
 *****************************************************************************/


Bool gpHci_CommandCompleteEvent(UInt8 eventCode, gpHci_CommandCompleteParams_t* pBuf)
{
/* <CodeGenerator Placeholder> gpHci_CommandCompleteHandler_AdditionalManual */
    Bool txSuccess = false;
    UInt8 dataBuf[1 /* HCI CC event code */ + 1 /* len */ + 255 /* maxparamsize = [ numcmdpkts, opcode, returnparams ] */];
    UInt8* ptr;
    ptr = dataBuf;
    UInt8* pPoolMemBufferToRelease = NULL;

    // Build HCI Event Frame header
    dataBuf[0] = eventCode;
#define length_field dataBuf[1]
    dataBuf[2] = pBuf->numHciCmdPackets;
    MEMCPY(&dataBuf[3], &pBuf->opCode, sizeof(gpHci_CommandOpCode_t));
    dataBuf[5] = pBuf->result;

    // ptr to the rest of the event payload (i.e. the opcode dependant data)
    ptr = &dataBuf[6];

    switch(pBuf->opCode)
    {
        case gpHci_OpCodeLeReadBufferSize:
        {
            length_field = 4 + 3; /*FIXME padding: sizeof(gpHci_LEReadBufferSize_t) == 4*/ /* == total_event_frame_length - 2 */

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.leReadBufferSize.ACLDataPacketLength, sizeof(UInt16));
            ptr+=sizeof(UInt16);
            *ptr = pBuf->returnParams.leReadBufferSize.totalNumDataPackets;
            break;
        }
        case gpHci_OpCodeReadLocalVersionInformation:
        {
            length_field = 4 + 1 + 2 + 1 + 2 + 2;

            // Build HCI Command Complete Event payload
            *ptr++ = pBuf->returnParams.leReadLocalVersion.hciVersion;
            MEMCPY(ptr, &pBuf->returnParams.leReadLocalVersion.hciRevision, sizeof(UInt16));
            ptr+=sizeof(UInt16);
            *ptr++ = pBuf->returnParams.leReadLocalVersion.lmppalVersion;
            MEMCPY(ptr, &pBuf->returnParams.leReadLocalVersion.manufacturerName, sizeof(UInt16));
            ptr+=sizeof(UInt16);
            MEMCPY(ptr, &pBuf->returnParams.leReadLocalVersion.lmppalSubversion, sizeof(UInt16));
            break;
        }
        case gpHci_OpCodeReadLocalSupportedCommands:
        {
            length_field = 4 + 64;

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.supportedCommands.supportedCommands, 64);
            break;
        }
        case gpHci_OpCodeLeReadSupportedStates:      // fall through
        case gpHci_OpCodeLeReadLocalSupportedFeatures:  // fall through
        case gpHci_OpCodeReadLocalSupportedFeatures:
        {
            length_field = 4 + 8;

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.supportedFeatures.supportedFeatures, 8);
            break;
        }
        case gpHci_OpCodeReadBdAddr:
        case gpHci_OpCodeVsdGeneratePeerResolvableAddress:
        case gpHci_OpCodeVsdGenerateLocalResolvableAddress:
        {
            length_field = 4 + sizeof(BtDeviceAddress_t);/* == total_event_frame_length - 2 */

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.bdAddress, sizeof(BtDeviceAddress_t));
            break;
        }
        case gpHci_OpCodeLeRemoteConnectionParamRequestReply:    // fall through
        case gpHci_OpCodeLeRemoteConnectionParamRequestNegReply: // fall through
        case gpHci_OpCodeLeSetDataLength:
        case gpHci_OpCodeLeLongTermKeyRequestReply:
        case gpHci_OpCodeLeLongTermKeyRequestNegativeReply:
        case gpHci_OpCodeWriteAuthenticatedPayloadTO: // fall through
#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
        case gpHci_OpCodeLeSetConnectionCteTransmitParameters:
        case gpHci_OpCodeLeConnectionCteResponseEnable:
        case gpHci_OpCodeVsdUnsolicitedCteTxEnable:
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */
        {
            length_field = 4 + sizeof(gpHci_ConnectionHandle_t);/* == total_event_frame_length - 2 */

            // Build HCI Command Complete Event payload
            // copy connection id from event buffer
            MEMCPY(ptr, &pBuf->returnParams.connectionHandle, sizeof(gpHci_ConnectionHandle_t));
            break;
        }
        case gpHci_OpCodeLeReadSuggestedDefDataLength:
        {
            length_field = 1 + 2 + 1 + sizeof(gpHci_LeWriteSuggestedDefDataLengthCommand_t);/* == total_event_frame_length - 2 */

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.readSuggestedDefDataLength, sizeof(gpHci_LeWriteSuggestedDefDataLengthCommand_t));
            break;
        }
        case gpHci_OpCodeLeReadMaxDataLength:
        {
            length_field = 4 /* status */ + sizeof(gpHci_ReadMaxDataLength_t);/* == total_event_frame_length - 2 */

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.readMaxDataLength, sizeof(gpHci_ReadMaxDataLength_t));
            break;
        }
        case gpHci_OpCodeLeReadAdvertisingChannelTxPower:
        {
            length_field = 4 + 1;

            // Build HCI Command Complete Event payload
            *ptr=pBuf->returnParams.advChannelTxPower;
            break;
        }
        case gpHci_OpCodeLeReadFilterAcceptListSize:
        case gpHci_OpCodeLeReadResolvingListSize:
#ifdef GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
        case gpHci_OpCodeVsdReadResolvingListCurrentSize:
#endif // GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
        {
            length_field = 4 + 1;

            // Build HCI Command Complete Event payload
            *ptr=pBuf->returnParams.resolvingListSize;
            break;
        }
        case gpHci_OpCodeReadTransmitPowerLevel:
        case gpHci_OpCodeReadRSSI:            // fall through
        {
            length_field = 4 + 3;

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.advanceTxPower.connectionHandle, sizeof(gpHci_ConnectionHandle_t));
            ptr+= sizeof(gpHci_ConnectionHandle_t);
            *ptr=pBuf->returnParams.advanceTxPower.advancedTxPower;
            break;
        }
        case gpHci_OpCodeLeReadChannelMap:
        {
            length_field = 4 + 7;

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.leReadChannelMap.connectionHandle, sizeof(gpHci_ConnectionHandle_t));
            ptr+= sizeof(gpHci_ConnectionHandle_t);
            MEMCPY(ptr, pBuf->returnParams.leReadChannelMap.channelMap.channels, GP_HCI_CHANNEL_MAP_LENGTH);
            break;
        }
        case gpHci_OpCodeLeEncrypt:
        {
            length_field = 4 + GP_HCI_ENCRYPTION_DATA_LENGTH;

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, pBuf->returnParams.encryptedData.encryptedData, GP_HCI_ENCRYPTION_DATA_LENGTH);
            break;
        }
        case gpHci_OpCodeLeRand: // fall through
        {
            length_field = 4 + GP_HCI_RANDOM_DATA_LENGTH;

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, pBuf->returnParams.randData.randomNumber, GP_HCI_RANDOM_DATA_LENGTH);
            break;
        }
        case gpHci_OpCodeReadAuthenticatedPayloadTO:
        {
            length_field = 4 + 4;

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.readAuthenticatedPayloadTO.connectionHandle, sizeof(gpHci_ConnectionHandle_t));
            ptr+= sizeof(gpHci_ConnectionHandle_t);
            MEMCPY(ptr, &pBuf->returnParams.readAuthenticatedPayloadTO.authenticatedPayloadTO, sizeof(UInt16));
            break;
        }
        case gpHci_OpCodeVsdGenerateAccessAddress:
        {
            length_field = 4 + sizeof(gpBle_AccessAddress_t);

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.accessAddress, 4);
            break;
        }
        case gpHci_OpCodeLeTestEnd:
        {
            length_field = 4 + sizeof(UInt16);/* == total_event_frame_length - 2 */

            // Build HCI Command Complete Event payload
            MEMCPY(ptr, &pBuf->returnParams.testResult, sizeof(UInt16));
            break;
        }
        case gpHci_OpCodeLeReadPhy:
        {
            length_field = 4 + sizeof(gpHci_ConnectionHandle_t) + sizeof(gpHci_Phy_t) + sizeof(gpHci_Phy_t);

            // Build HCI Command Complete Event payload
            // copy testResult from event buffer
            HOST_TO_LITTLE_UINT16(&connectionComplete->connectionHandle);
            MEMCPY(ptr, &pBuf->returnParams.leReadPhy.connectionHandle, sizeof(gpHci_ConnectionHandle_t));
            LITTLE_TO_HOST_UINT16(&connectionComplete->connectionHandle);
            ptr+=sizeof(gpHci_ConnectionHandle_t);
            *ptr++ = pBuf->returnParams.leReadPhy.txPhy;
            *ptr++ = pBuf->returnParams.leReadPhy.rxPhy;
            break;
        }
#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
        case gpHci_OpCodeLeReadAntennaInformation:
        {
            length_field = 4 + 1 + 1 + 1 + 1;

            // Build HCI Command Complete Event payload
            // copy testResult from event buffer
            *ptr++ = pBuf->returnParams.leReadAntennaInformation.supportedSwitchingSamplingRates;
            *ptr++ = pBuf->returnParams.leReadAntennaInformation.nrOfAntennae;
            *ptr++ = pBuf->returnParams.leReadAntennaInformation.maxSwitchPatternLength;
            *ptr++ = pBuf->returnParams.leReadAntennaInformation.maxCteLength;
            break;
        }
#endif // GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
        case gpHci_OpCodeLeReadTransmitPower:
        {
            length_field = 4 /* 1B numcmndpkts, 2B opcode, 1B status */ + 1 + 1;
            *ptr++ = pBuf->returnParams.LeReadTransmitPower.Min_Tx_Power;
            *ptr++ = pBuf->returnParams.LeReadTransmitPower.Max_Tx_Power;
            break;
        }

        case gpHci_OpCodeLeReadRfPathCompensation:
        {
            length_field = 4 /* 1B numcmndpkts, 2B opcode, 1B status */ + 2 + 2;
            MEMCPY(&dataBuf[6], &pBuf->returnParams.LeReadRfPathCompensation.RF_Tx_Path_Compensation_Value, sizeof(Int16));
            MEMCPY(&dataBuf[8], &pBuf->returnParams.LeReadRfPathCompensation.RF_Rx_Path_Compensation_Value, sizeof(Int16));
            break;
        }
#ifdef GP_DIVERSITY_DEVELOPMENT
        case gpHci_OpCodeVsdGetBuildP4Changelist:
        {
            length_field = 4 + sizeof(UInt32);

            // Build HCI Command Complete Event payload
            MEMCPY(&dataBuf[6], &(pBuf->returnParams.vsdGetBuildP4Changelist.changelistNumber), sizeof(UInt32));
            break;
        }
#endif // GP_DIVERSITY_DEVELOPMENT
#ifdef GP_DIVERSITY_DEVELOPMENT
        case gpHci_OpCodeVsdRNG:
        {
            UInt8 num_rngbytes = pBuf->returnParams.VsdRandData.numRandomBytes;
            length_field = 4 /* 1B numcmndpkts, 2B opcode, 1B status */ + 1 + 1 + num_rngbytes;
            *ptr++ = pBuf->returnParams.VsdRandData.RNG_Source;
            *ptr++ = num_rngbytes;
            MEMCPY(ptr, pBuf->returnParams.VsdRandData.pRandomData, num_rngbytes);
            pPoolMemBufferToRelease = pBuf->returnParams.VsdRandData.pRandomData;
            break;
        }
#endif // GP_DIVERSITY_DEVELOPMENT
#ifdef GP_DIVERSITY_DEVELOPMENT
        case gpHci_OpCodeVsdGetRtMgrVersion:
        {
            length_field = 4 + sizeof(UInt8);


            *ptr++ = pBuf->returnParams.RtMgrVersion.version;
            break;
        }
#endif // GP_DIVERSITY_DEVELOPMENT
        default:
        {
            length_field = 4; // length of event payload: command commplete without extra payload

            // Build HCI Command Complete Event payload
            break;
        }
    }

    txSuccess = DATA_REQUEST( 1 /* HCI CC event code */ + 1 /* len */ + length_field , dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT );
#undef length_field

    if ( txSuccess &&
         (NULL != pPoolMemBufferToRelease)
       )
    {
        gpPoolMem_Free(pPoolMemBufferToRelease);
    }

    return txSuccess;
/* </CodeGenerator Placeholder> gpHci_CommandCompleteHandler_AdditionalManual */
}

#ifndef GP_HCI_DIVERSITY_HOST_SERVER
Bool gpHci_CommandStatusEvent(gpHci_EventCode_t eventCode, gpHci_CommandStatusParams_t* pBuf)
{
    UInt8 dataBuf[6];
    dataBuf[0] = eventCode;
    dataBuf[1] = 4; /* Command Status events have a fixed length*/
    dataBuf[2] = pBuf->status;
    dataBuf[3] = pBuf->numHciCmdPackets;
    MEMCPY( &dataBuf[4], &pBuf->opCode, sizeof(gpHci_CommandOpCode_t));

    return DATA_REQUEST( 6 , dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT );
}


Bool gpHci_ConnectionCompleteEvent(UInt8 eventCode, gpHci_ConnectionCompleteParams_t* connectionComplete)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + (1 + 2*1 + 6*1 + 1 + 1)];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define connectionCompletePacket                            (&dataBuf[2])
#define connectionCompletePacket_status                     dataBuf[2]
#define connectionCompletePacket_connectionHandle           dataBuf[2 + 1]
#define connectionCompletePacket_bdAddress                  dataBuf[2 + 1 + 2*1]
#define connectionCompletePacket_linkType                   dataBuf[2 + 1 + 2*1 + 6*1]
#define connectionCompletePacket_encryptionEnabled          dataBuf[2 + 1 + 2*1 + 6*1 + 1]

    eventCodePacket = eventCode;
    lengthPacket = length;

        HOST_TO_LITTLE_UINT16(&connectionComplete->connectionHandle);
        connectionCompletePacket_status = connectionComplete->status;
        MEMCPY(&(connectionCompletePacket_connectionHandle), (UInt8*)&(connectionComplete->connectionHandle), 2*1);
        MEMCPY(&(connectionCompletePacket_bdAddress), (UInt8*)&(connectionComplete->bdAddress), 6*1);
        connectionCompletePacket_linkType = connectionComplete->linkType;
        connectionCompletePacket_encryptionEnabled = connectionComplete->encryptionEnabled;
        LITTLE_TO_HOST_UINT16(&connectionComplete->connectionHandle);



    lengthPacket = 13 - 2;
    retVal = DATA_REQUEST(13, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef connectionCompletePacket
#undef connectionCompletePacket_status
#undef connectionCompletePacket_connectionHandle
#undef connectionCompletePacket_bdAddress
#undef connectionCompletePacket_linkType
#undef connectionCompletePacket_encryptionEnabled
}

Bool gpHci_DisconnectionCompleteEvent(UInt8 eventCode, gpHci_DisconnectCompleteParams_t* disconnectComplete)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + (1 + 2*1 + 1)];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define disconnectCompletePacket                            (&dataBuf[2])
#define disconnectCompletePacket_status                     dataBuf[2]
#define disconnectCompletePacket_connectionHandle           dataBuf[2 + 1]
#define disconnectCompletePacket_reason                     dataBuf[2 + 1 + 2*1]

    eventCodePacket = eventCode;
    lengthPacket = length;

        HOST_TO_LITTLE_UINT16(&disconnectComplete->connectionHandle);
        disconnectCompletePacket_status = disconnectComplete->status;
        MEMCPY(&(disconnectCompletePacket_connectionHandle), (UInt8*)&(disconnectComplete->connectionHandle), 2*1);
        disconnectCompletePacket_reason = disconnectComplete->reason;
        LITTLE_TO_HOST_UINT16(&disconnectComplete->connectionHandle);



    lengthPacket = 6 - 2;
    retVal = DATA_REQUEST(6, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef disconnectCompletePacket
#undef disconnectCompletePacket_status
#undef disconnectCompletePacket_connectionHandle
#undef disconnectCompletePacket_reason
}

Bool gpHci_EncryptionChangeEvent(UInt8 eventCode, gpHci_EncryptionChangeParams_t* encryptionChange)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + (1 + 2*1 + 1)];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define encryptionChangePacket                              (&dataBuf[2])
#define encryptionChangePacket_status                       dataBuf[2]
#define encryptionChangePacket_connectionHandle             dataBuf[2 + 1]
#define encryptionChangePacket_encryptionEnabled            dataBuf[2 + 1 + 2*1]

    eventCodePacket = eventCode;
    lengthPacket = length;

        HOST_TO_LITTLE_UINT16(&encryptionChange->connectionHandle);
        encryptionChangePacket_status = encryptionChange->status;
        MEMCPY(&(encryptionChangePacket_connectionHandle), (UInt8*)&(encryptionChange->connectionHandle), 2*1);
        encryptionChangePacket_encryptionEnabled = encryptionChange->encryptionEnabled;
        LITTLE_TO_HOST_UINT16(&encryptionChange->connectionHandle);



    lengthPacket = 6 - 2;
    retVal = DATA_REQUEST(6, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef encryptionChangePacket
#undef encryptionChangePacket_status
#undef encryptionChangePacket_connectionHandle
#undef encryptionChangePacket_encryptionEnabled
}

Bool gpHci_ReadRemoteVersionCompleteEvent(UInt8 eventCode, gpHci_ReadRemoteVersionInfoComplete_t* versionInfo)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + (1 + 2*1 + 1 + 2*1 + 2*1)];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define versionInfoPacket                                   (&dataBuf[2])
#define versionInfoPacket_status                            dataBuf[2]
#define versionInfoPacket_connectionHandle                  dataBuf[2 + 1]
#define versionInfoPacket_versionNr                         dataBuf[2 + 1 + 2*1]
#define versionInfoPacket_companyId                         dataBuf[2 + 1 + 2*1 + 1]
#define versionInfoPacket_subVersionNr                      dataBuf[2 + 1 + 2*1 + 1 + 2*1]

    eventCodePacket = eventCode;
    lengthPacket = length;

        HOST_TO_LITTLE_UINT16(&versionInfo->connectionHandle);
        HOST_TO_LITTLE_UINT16(&versionInfo->companyId);
        HOST_TO_LITTLE_UINT16(&versionInfo->subVersionNr);
        versionInfoPacket_status = versionInfo->status;
        MEMCPY(&(versionInfoPacket_connectionHandle), (UInt8*)&(versionInfo->connectionHandle), 2*1);
        versionInfoPacket_versionNr = versionInfo->versionNr;
        MEMCPY(&(versionInfoPacket_companyId), (UInt8*)&(versionInfo->companyId), 2*1);
        MEMCPY(&(versionInfoPacket_subVersionNr), (UInt8*)&(versionInfo->subVersionNr), 2*1);
        LITTLE_TO_HOST_UINT16(&versionInfo->connectionHandle);
        LITTLE_TO_HOST_UINT16(&versionInfo->companyId);
        LITTLE_TO_HOST_UINT16(&versionInfo->subVersionNr);



    lengthPacket = 10 - 2;
    retVal = DATA_REQUEST(10, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef versionInfoPacket
#undef versionInfoPacket_status
#undef versionInfoPacket_connectionHandle
#undef versionInfoPacket_versionNr
#undef versionInfoPacket_companyId
#undef versionInfoPacket_subVersionNr
}

Bool gpHci_HardwareErrorEvent(UInt8 eventCode, UInt8 hwcode)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define hwcodePacket                                        dataBuf[2]

    eventCodePacket = eventCode;
    lengthPacket = length;
    hwcodePacket = hwcode;


    lengthPacket = 3 - 2;
    retVal = DATA_REQUEST(3, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef hwcodePacket
}

Bool gpHci_NumberOfCompletedPacketsEvent(UInt8 eventCode, gpHci_NumberOfCompletedPackets_t* numberOfCompletedPackets)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + (1 + 2*1 + 2*1)];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define numberOfCompletedPacketsPacket                  (&dataBuf[2])
#define numberOfCompletedPacketsPacket_nrOfHandles      dataBuf[2]
#define numberOfCompletedPacketsPacket_handle           dataBuf[2 + 1]
#define numberOfCompletedPacketsPacket_nrOfHciPackets   dataBuf[2 + 1 + 2*1]

    eventCodePacket = eventCode;
    lengthPacket = length;

        HOST_TO_LITTLE_UINT16(&numberOfCompletedPackets->handle);
        HOST_TO_LITTLE_UINT16(&numberOfCompletedPackets->nrOfHciPackets);
        numberOfCompletedPacketsPacket_nrOfHandles = numberOfCompletedPackets->nrOfHandles;
        MEMCPY(&(numberOfCompletedPacketsPacket_handle), (UInt8*)&(numberOfCompletedPackets->handle), 2*1);
        MEMCPY(&(numberOfCompletedPacketsPacket_nrOfHciPackets), (UInt8*)&(numberOfCompletedPackets->nrOfHciPackets), 2*1);
        LITTLE_TO_HOST_UINT16(&numberOfCompletedPackets->handle);
        LITTLE_TO_HOST_UINT16(&numberOfCompletedPackets->nrOfHciPackets);



    lengthPacket = 7 - 2;
    retVal = DATA_REQUEST(7, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef numberOfCompletedPacketsPacket
#undef numberOfCompletedPacketsPacket_nrOfHandles
#undef numberOfCompletedPacketsPacket_handle
#undef numberOfCompletedPacketsPacket_nrOfHciPackets
}

Bool gpHci_DataBufferOverflowEvent(UInt8 eventCode, gpHci_LinkType_t linktype)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + 1];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define linktypePacket                                      dataBuf[2]

    eventCodePacket = eventCode;
    lengthPacket = length;
    linktypePacket = linktype;


    lengthPacket = 3 - 2;
    retVal = DATA_REQUEST(3, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef linktypePacket
}

Bool gpHci_EncryptionKeyRefreshCompleteEvent(UInt8 eventCode, gpHci_EncryptionKeyRefreshComplete_t* keyRefresh)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + (1 + 2*1)];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define keyRefreshPacket                                    (&dataBuf[2])
#define keyRefreshPacket_status                             dataBuf[2]
#define keyRefreshPacket_connectionHandle                   dataBuf[2 + 1]

    eventCodePacket = eventCode;
    lengthPacket = length;

        HOST_TO_LITTLE_UINT16(&keyRefresh->connectionHandle);
        keyRefreshPacket_status = keyRefresh->status;
        MEMCPY(&(keyRefreshPacket_connectionHandle), (UInt8*)&(keyRefresh->connectionHandle), 2*1);
        LITTLE_TO_HOST_UINT16(&keyRefresh->connectionHandle);



    lengthPacket = 5 - 2;
    retVal = DATA_REQUEST(5, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef keyRefreshPacket
#undef keyRefreshPacket_status
#undef keyRefreshPacket_connectionHandle
}

Bool gpHci_AuthenticationPayloadTOEvent(UInt8 eventCode, gpHci_AuthenticatedPayloadToExpired_t* authenticatedPayloadToExpired)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + (2*1)];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define authenticatedPayloadToExpiredPacket                 (&dataBuf[2])
#define authenticatedPayloadToExpiredPacket_connectionHandle dataBuf[2]

    eventCodePacket = eventCode;
    lengthPacket = length;

        HOST_TO_LITTLE_UINT16(&authenticatedPayloadToExpired->connectionHandle);
        MEMCPY(&(authenticatedPayloadToExpiredPacket_connectionHandle), (UInt8*)&(authenticatedPayloadToExpired->connectionHandle), 2*1);
        LITTLE_TO_HOST_UINT16(&authenticatedPayloadToExpired->connectionHandle);



    lengthPacket = 4 - 2;
    retVal = DATA_REQUEST(4, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef authenticatedPayloadToExpiredPacket
#undef authenticatedPayloadToExpiredPacket_connectionHandle
}

Bool gpHci_SendHciDataFrameToHost(UInt16 connAndBoundary, UInt16 dataLength, UInt8* pData)
{
/* <CodeGenerator Placeholder> gpHci_SendHciDataFrameToHost_AdditionalManual */
    UInt16 length = dataLength - 4; /* Datalength is actually the hci packet length*/

#define connAndBoundaryPacket                               pData[0]
#define dataLengthPacket                                    pData[2]
#define pDataPacket                                         (&pData[4])

    HOST_TO_LITTLE_UINT16(&connAndBoundary);
    MEMCPY(&connAndBoundaryPacket, (UInt8*)&connAndBoundary, sizeof(UInt16));
    LITTLE_TO_HOST_UINT16(&connAndBoundary);
    HOST_TO_LITTLE_UINT16(&length);
    MEMCPY(&dataLengthPacket, (UInt8*)&length, sizeof(UInt16));
    LITTLE_TO_HOST_UINT16(&length);

    return DATA_REQUEST(dataLength, pData, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_DATA);
#undef connAndBoundaryPacket
#undef dataLengthPacket
#undef pDataPacket
/* </CodeGenerator Placeholder> gpHci_SendHciDataFrameToHost_AdditionalManual */
}


Bool gpHci_VsdSinkRxIndication(UInt8 eventCode, gpHci_VsdSinkRxIndication_t* vsdSinkRxIndication)
{

    UInt8 length = 0;

    Bool retVal;
    UInt8 dataBuf[1 + 1 + 1 + (2*1 + 2*1 + 4*1)];

#define eventCodePacket                                     dataBuf[0]
#define lengthPacket                                        dataBuf[1]
#define vsdSinkRxIndicationPacket                           (&dataBuf[2])
#define vsdSinkRxIndicationPacket_connHandle                dataBuf[2]
#define vsdSinkRxIndicationPacket_dataLength                dataBuf[2 + 2*1]
#define vsdSinkRxIndicationPacket_rxTs                      dataBuf[2 + 2*1 + 2*1]

    eventCodePacket = eventCode;
    lengthPacket = length;

        HOST_TO_LITTLE_UINT16(&vsdSinkRxIndication->connHandle);
        HOST_TO_LITTLE_UINT16(&vsdSinkRxIndication->dataLength);
        HOST_TO_LITTLE_UINT32(&vsdSinkRxIndication->rxTs);
        MEMCPY(&(vsdSinkRxIndicationPacket_connHandle), (UInt8*)&(vsdSinkRxIndication->connHandle), 2*1);
        MEMCPY(&(vsdSinkRxIndicationPacket_dataLength), (UInt8*)&(vsdSinkRxIndication->dataLength), 2*1);
        MEMCPY(&(vsdSinkRxIndicationPacket_rxTs), (UInt8*)&(vsdSinkRxIndication->rxTs), 4*1);
        LITTLE_TO_HOST_UINT16(&vsdSinkRxIndication->connHandle);
        LITTLE_TO_HOST_UINT16(&vsdSinkRxIndication->dataLength);
        LITTLE_TO_HOST_UINT32(&vsdSinkRxIndication->rxTs);



    lengthPacket = 10 - 2;
    retVal = DATA_REQUEST(10, dataBuf, GP_HCI_COMM_ID | GP_COM_COMM_ID_BLE_EVENT);
    return retVal;

#undef eventCodePacket
#undef lengthPacket
#undef vsdSinkRxIndicationPacket
#undef vsdSinkRxIndicationPacket_connHandle
#undef vsdSinkRxIndicationPacket_dataLength
#undef vsdSinkRxIndicationPacket_rxTs
}
#endif // GP_HCI_DIVERSITY_HOST_SERVER
