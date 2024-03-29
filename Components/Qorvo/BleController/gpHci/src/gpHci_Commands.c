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
#ifdef GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED
#include "gpBleConnectionManager.h"
#endif //GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED
#include "gpBleAddressResolver.h"
#include "gpBleConfig.h"
#include "gpBleSecurityCoprocessor.h"
#ifdef GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
#include "gpBleTestMode.h"
#endif // GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
#ifdef GP_COMP_BLEDIRECTIONFINDING
#include "gpBleDirectionFinding.h"
#include "gpBleLlcpProcedures_ConstantToneExtension.h"
#endif //GP_COMP_BLEDIRECTIONFINDING
#include "gpPoolMem.h"
#include "gpHci_defs.h"
#ifdef GP_COMP_BLERESPRADDR
#include "gpBleResPrAddr.h"
#endif // GP_COMP_BLERESPRADDR
#include "gpBleComps.h"
#include "gpBlePreSched.h"
#if defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleAdvertiser.h"
#endif //GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_PERIPHERAL
#ifdef GP_COMP_BLESCANNER
#include "gpBleScanner.h"
#endif //GP_COMP_BLESCANNER
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
#include "gpBleLlcpProcedures.h"
#include "gpBleInitiator.h"
#include "gpBleDataCommon.h"
#include "gpBleDataRx.h"
#include "gpBleLlcp.h"
#endif //GP_DIVERSITY_BLE_CENTRAL || GP_DIVERSITY_BLE_PERIPHERAL
/* </CodeGenerator Placeholder> AdditionalIncludes */

/*****************************************************************************
 *                    Typedef Definition
 *****************************************************************************/

/*****************************************************************************
*                    Static Functions Declaration
*****************************************************************************/

/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_HCI

/*****************************************************************************
 *                    Function Definitions
 *****************************************************************************/

void gpHci_processCommand(UInt16 OpCode, UInt8 totalLength, UInt8* pPayload)
{

    gpHci_CommandParameters_t    command;
    gpBle_ActionFunc_t          function;
    MEMSET(&command, 0, sizeof(gpHci_CommandParameters_t));
    function=NULL;

    GP_LOG_PRINTF(" opcode 0x%lx len 0x%lx  ",6, (long unsigned int) OpCode,(long unsigned int) totalLength);

    switch(OpCode)
    {
#if defined(GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED)
        case gpHci_OpCodeDisconnect:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.Disconnect.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _reason                                                       pPayload[0 + 2*1]
            command.Disconnect.reason = _reason;
#undef _reason
            function = gpBle_Disconnect;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeReadRemoteVersionInfo:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.ReadRemoteVersionInfo.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
            function = gpBle_ReadRemoteVersionInfo;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
        case gpHci_OpCodeSetEventMask:
        {
#define _eventMask                                                    pPayload[0]
            MEMCPY(&command.SetEventMask.eventMask, &_eventMask, sizeof(gpHci_EventMask_t));
#undef _eventMask
            function = gpBle_SetEventMask;
            break;
        }
        case gpHci_OpCodeReset:
        {
            function = gpBle_Reset;
            break;
        }
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeReadTransmitPowerLevel:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.ReadTransmitPowerLevel.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _type                                                         pPayload[0 + 2*1]
            command.ReadTransmitPowerLevel.type = _type;
#undef _type
            function = gpBle_ReadTransmitPowerLevel;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
        case gpHci_OpCodeSetEventMaskPage2:
        {
#define _eventMask                                                    pPayload[0]
            MEMCPY(&command.SetEventMaskPage2.eventMask, &_eventMask, sizeof(gpHci_EventMask_t));
#undef _eventMask
            function = gpBle_SetEventMaskPage2;
            break;
        }
#if defined(GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED)
        case gpHci_OpCodeReadAuthenticatedPayloadTO:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.ReadAuthenticatedPayloadTO.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
            function = gpBle_ReadAuthenticatedPayloadTO;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED)
        case gpHci_OpCodeWriteAuthenticatedPayloadTO:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.WriteAuthenticatedPayloadTO.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _authenticatedPayloadTO                                       pPayload[0 + 2*1]
            MEMCPY(&command.WriteAuthenticatedPayloadTO.authenticatedPayloadTO, &_authenticatedPayloadTO, sizeof(UInt16));
#undef _authenticatedPayloadTO
            function = gpBle_WriteAuthenticatedPayloadTO;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED) */
        case gpHci_OpCodeReadLocalVersionInformation:
        {
            function = gpBle_ReadLocalVersionInformation;
            break;
        }
        case gpHci_OpCodeReadLocalSupportedCommands:
        {
            function = gpBle_ReadLocalSupportedCommands;
            break;
        }
        case gpHci_OpCodeReadLocalSupportedFeatures:
        {
            function = gpBle_ReadLocalSupportedFeatures;
            break;
        }
        case gpHci_OpCodeReadBufferSize:
        {
            function = gpBle_ReadBufferSize;
            break;
        }
        case gpHci_OpCodeReadBdAddr:
        {
            function = gpBle_ReadBdAddr;
            break;
        }
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeReadRSSI:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.ReadRSSI.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
            function = gpBle_ReadRSSI;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
        case gpHci_OpCodeLeSetEventMask:
        {
#define _eventMask                                                    pPayload[0]
            MEMCPY(&command.LeSetEventMask.eventMask, &_eventMask, sizeof(gpHci_EventMask_t));
#undef _eventMask
            function = gpBle_LeSetEventMask;
            break;
        }
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeLeReadBufferSize:
        {
            function = gpBle_LeReadBufferSize;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
        case gpHci_OpCodeLeReadLocalSupportedFeatures:
        {
            function = gpBle_LeReadLocalSupportedFeatures;
            break;
        }
        case gpHci_OpCodeLeSetRandomAddress:
        {
#define _address                                                      pPayload[0]
            MEMCPY(&command.LeSetRandomAddress.address, &_address, sizeof(BtDeviceAddress_t));
#undef _address
            function = gpBle_LeSetRandomAddress;
            break;
        }
#if defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER)
        case gpHci_OpCodeLeSetAdvertisingParameters:
        {
#define _advertisingIntervalMin                                       pPayload[0]
            MEMCPY(&command.LeSetAdvertisingParameters.advertisingIntervalMin, &_advertisingIntervalMin, sizeof(UInt16));
#undef _advertisingIntervalMin
#define _advertisingIntervalMax                                       pPayload[0 + 2*1]
            MEMCPY(&command.LeSetAdvertisingParameters.advertisingIntervalMax, &_advertisingIntervalMax, sizeof(UInt16));
#undef _advertisingIntervalMax
#define _advertisingType                                              pPayload[0 + 2*1 + 2*1]
            command.LeSetAdvertisingParameters.advertisingType = _advertisingType;
#undef _advertisingType
#define _ownAddressType                                               pPayload[0 + 2*1 + 2*1 + 1]
            command.LeSetAdvertisingParameters.ownAddressType = _ownAddressType;
#undef _ownAddressType
#define _peerAddressType                                              pPayload[0 + 2*1 + 2*1 + 1 + 1]
            command.LeSetAdvertisingParameters.peerAddressType = _peerAddressType;
#undef _peerAddressType
#define _peerAddress                                                  pPayload[0 + 2*1 + 2*1 + 1 + 1 + 1]
            MEMCPY(&command.LeSetAdvertisingParameters.peerAddress, &_peerAddress, sizeof(BtDeviceAddress_t));
#undef _peerAddress
#define _channelMap                                                   pPayload[0 + 2*1 + 2*1 + 1 + 1 + 1 + 6*1]
            command.LeSetAdvertisingParameters.channelMap = _channelMap;
#undef _channelMap
#define _filterPolicy                                                 pPayload[0 + 2*1 + 2*1 + 1 + 1 + 1 + 6*1 + 1]
            command.LeSetAdvertisingParameters.filterPolicy = _filterPolicy;
#undef _filterPolicy
            function = gpBle_LeSetAdvertisingParameters;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER)
        case gpHci_OpCodeLeReadAdvertisingChannelTxPower:
        {
            function = gpBle_LeReadAdvertisingChannelTxPower;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER)
        case gpHci_OpCodeLeSetAdvertisingData:
        {
#define _length                                                       pPayload[0]
            command.LeSetAdvertisingData.length = _length;
#undef _length
#define _data                                                         (((UInt8*) (&pPayload[0 + 1])))
            command.LeSetAdvertisingData.data = _data;
#undef _data
            function = gpBle_LeSetAdvertisingData;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER)
        case gpHci_OpCodeLeSetScanResponseData:
        {
#define _length                                                       pPayload[0]
            command.LeSetScanResponseData.length = _length;
#undef _length
#define _data                                                         (((UInt8*) (&pPayload[0 + 1])))
            command.LeSetScanResponseData.data = _data;
#undef _data
            function = gpBle_LeSetScanResponseData;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER)
        case gpHci_OpCodeLeSetAdvertiseEnable:
        {
#define _enable                                                       pPayload[0]
            command.LeSetAdvertiseEnable.enable = _enable;
#undef _enable
            function = gpBle_LeSetAdvertiseEnable;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_BROADCASTER) */
#if defined(GP_DIVERSITY_BLE_OBSERVER)
        case gpHci_OpCodeLeSetScanParameters:
        {
#define _scanType                                                     pPayload[0]
            command.LeSetScanParameters.scanType = _scanType;
#undef _scanType
#define _scanInterval                                                 pPayload[0 + 1]
            MEMCPY(&command.LeSetScanParameters.scanInterval, &_scanInterval, sizeof(UInt16));
#undef _scanInterval
#define _scanWindow                                                   pPayload[0 + 1 + 2*1]
            MEMCPY(&command.LeSetScanParameters.scanWindow, &_scanWindow, sizeof(UInt16));
#undef _scanWindow
#define _ownAddressType                                               pPayload[0 + 1 + 2*1 + 2*1]
            command.LeSetScanParameters.ownAddressType = _ownAddressType;
#undef _ownAddressType
#define _filterPolicy                                                 pPayload[0 + 1 + 2*1 + 2*1 + 1]
            command.LeSetScanParameters.filterPolicy = _filterPolicy;
#undef _filterPolicy
            function = gpBle_LeSetScanParameters;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_OBSERVER) */
#if defined(GP_DIVERSITY_BLE_OBSERVER)
        case gpHci_OpCodeLeSetScanEnable:
        {
#define _enable                                                       pPayload[0]
            command.LeSetScanEnable.enable = _enable;
#undef _enable
#define _filterDuplicates                                             pPayload[0 + 1]
            command.LeSetScanEnable.filterDuplicates = _filterDuplicates;
#undef _filterDuplicates
            function = gpBle_LeSetScanEnable;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_OBSERVER) */
        case gpHci_OpCodeLeReadFilterAcceptListSize:
        {
            function = gpBle_LeReadFilterAcceptListSize;
            break;
        }
        case gpHci_OpCodeLeClearFilterAcceptList:
        {
            function = gpBle_LeClearFilterAcceptList;
            break;
        }
        case gpHci_OpCodeLeAddDeviceToFilterAcceptList:
        {
#define _addressType                                                  pPayload[0]
            command.LeAddDeviceToFilterAcceptList.addressType = _addressType;
#undef _addressType
#define _address                                                      pPayload[0 + 1]
            MEMCPY(&command.LeAddDeviceToFilterAcceptList.address, &_address, sizeof(BtDeviceAddress_t));
#undef _address
            function = gpBle_LeAddDeviceToFilterAcceptList;
            break;
        }
        case gpHci_OpCodeLeRemoveDeviceFromFilterAcceptList:
        {
#define _addressType                                                  pPayload[0]
            command.LeRemoveDeviceFromFilterAcceptList.addressType = _addressType;
#undef _addressType
#define _address                                                      pPayload[0 + 1]
            MEMCPY(&command.LeRemoveDeviceFromFilterAcceptList.address, &_address, sizeof(BtDeviceAddress_t));
#undef _address
            function = gpBle_LeRemoveDeviceFromFilterAcceptList;
            break;
        }
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeLeConnectionUpdate:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeConnectionUpdate.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _connIntervalMin                                              pPayload[0 + 2*1]
            MEMCPY(&command.LeConnectionUpdate.connIntervalMin, &_connIntervalMin, sizeof(UInt16));
#undef _connIntervalMin
#define _connIntervalMax                                              pPayload[0 + 2*1 + 2*1]
            MEMCPY(&command.LeConnectionUpdate.connIntervalMax, &_connIntervalMax, sizeof(UInt16));
#undef _connIntervalMax
#define _connLatency                                                  pPayload[0 + 2*1 + 2*1 + 2*1]
            MEMCPY(&command.LeConnectionUpdate.connLatency, &_connLatency, sizeof(UInt16));
#undef _connLatency
#define _supervisionTimeout                                           pPayload[0 + 2*1 + 2*1 + 2*1 + 2*1]
            MEMCPY(&command.LeConnectionUpdate.supervisionTimeout, &_supervisionTimeout, sizeof(UInt16));
#undef _supervisionTimeout
#define _minCELength                                                  pPayload[0 + 2*1 + 2*1 + 2*1 + 2*1 + 2*1]
            MEMCPY(&command.LeConnectionUpdate.minCELength, &_minCELength, sizeof(UInt16));
#undef _minCELength
#define _maxCELength                                                  pPayload[0 + 2*1 + 2*1 + 2*1 + 2*1 + 2*1 + 2*1]
            MEMCPY(&command.LeConnectionUpdate.maxCELength, &_maxCELength, sizeof(UInt16));
#undef _maxCELength
            function = gpBle_LeConnectionUpdate;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeLeReadChannelMap:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeReadChannelMap.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
            function = gpBle_LeReadChannelMap;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeLeReadRemoteFeatures:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeReadRemoteFeatures.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
            function = gpBle_LeReadRemoteFeatures;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
#if defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED)
        case gpHci_OpCodeLeEncrypt:
        {
#define _key                                                          (((UInt8*) (&pPayload[0])))
            command.LeEncrypt.key = _key;
#undef _key
#define _data                                                         (((UInt8*) (&pPayload[0 + 16])))
            command.LeEncrypt.data = _data;
#undef _data
            function = gpBle_LeEncrypt;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED)
        case gpHci_OpCodeLeRand:
        {
            function = gpBle_LeRand;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED)
        case gpHci_OpCodeLeLongTermKeyRequestReply:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeLongTermKeyRequestReply.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _longTermKey                                                  (((UInt8*) (&pPayload[0 + 2*1])))
            command.LeLongTermKeyRequestReply.longTermKey = _longTermKey;
#undef _longTermKey
            function = gpBle_LeLongTermKeyRequestReply;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED)
        case gpHci_OpCodeLeLongTermKeyRequestNegativeReply:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeLongTermKeyRequestNegativeReply.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
            function = gpBle_LeLongTermKeyRequestNegativeReply;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED) */
        case gpHci_OpCodeLeReadSupportedStates:
        {
            function = gpBle_LeReadSupportedStates;
            break;
        }
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeLeReceiverTest:
        {
#define _rxchannel                                                    pPayload[0]
            command.LeReceiverTest.rxchannel = _rxchannel;
#undef _rxchannel
            function = gpBle_LeReceiverTest;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeLeTransmitterTest:
        {
#define _txchannel                                                    pPayload[0]
            command.LeTransmitterTest.txchannel = _txchannel;
#undef _txchannel
#define _length                                                       pPayload[0 + 1]
            command.LeTransmitterTest.length = _length;
#undef _length
#define _payload                                                      pPayload[0 + 1 + 1]
            command.LeTransmitterTest.payload = _payload;
#undef _payload
            function = gpBle_LeTransmitterTest;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeLeTestEnd:
        {
            function = gpBle_LeTestEnd;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED)
        case gpHci_OpCodeLeRemoteConnectionParamRequestReply:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeRemoteConnectionParamRequestReply.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _connIntervalMin                                              pPayload[0 + 2*1]
            MEMCPY(&command.LeRemoteConnectionParamRequestReply.connIntervalMin, &_connIntervalMin, sizeof(UInt16));
#undef _connIntervalMin
#define _connIntervalMax                                              pPayload[0 + 2*1 + 2*1]
            MEMCPY(&command.LeRemoteConnectionParamRequestReply.connIntervalMax, &_connIntervalMax, sizeof(UInt16));
#undef _connIntervalMax
#define _connLatency                                                  pPayload[0 + 2*1 + 2*1 + 2*1]
            MEMCPY(&command.LeRemoteConnectionParamRequestReply.connLatency, &_connLatency, sizeof(UInt16));
#undef _connLatency
#define _supervisionTimeout                                           pPayload[0 + 2*1 + 2*1 + 2*1 + 2*1]
            MEMCPY(&command.LeRemoteConnectionParamRequestReply.supervisionTimeout, &_supervisionTimeout, sizeof(UInt16));
#undef _supervisionTimeout
#define _minCELength                                                  pPayload[0 + 2*1 + 2*1 + 2*1 + 2*1 + 2*1]
            MEMCPY(&command.LeRemoteConnectionParamRequestReply.minCELength, &_minCELength, sizeof(UInt16));
#undef _minCELength
#define _maxCELength                                                  pPayload[0 + 2*1 + 2*1 + 2*1 + 2*1 + 2*1 + 2*1]
            MEMCPY(&command.LeRemoteConnectionParamRequestReply.maxCELength, &_maxCELength, sizeof(UInt16));
#undef _maxCELength
            function = gpBle_LeRemoteConnectionParamRequestReply;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED)
        case gpHci_OpCodeLeRemoteConnectionParamRequestNegReply:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeRemoteConnectionParamRequestNegReply.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _reason                                                       pPayload[0 + 2*1]
            command.LeRemoteConnectionParamRequestNegReply.reason = _reason;
#undef _reason
            function = gpBle_LeRemoteConnectionParamRequestNegReply;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED)
        case gpHci_OpCodeLeSetDataLength:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeSetDataLength.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _txOctets                                                     pPayload[0 + 2*1]
            MEMCPY(&command.LeSetDataLength.txOctets, &_txOctets, sizeof(UInt16));
#undef _txOctets
#define _txTime                                                       pPayload[0 + 2*1 + 2*1]
            MEMCPY(&command.LeSetDataLength.txTime, &_txTime, sizeof(UInt16));
#undef _txTime
            function = gpBle_LeSetDataLength;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED)
        case gpHci_OpCodeLeReadSuggestedDefDataLength:
        {
            function = gpBle_LeReadSuggestedDefDataLength;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED)
        case gpHci_OpCodeLeWriteSuggestedDefDataLength:
        {
#define _suggestedMaxTxOctets                                         pPayload[0]
            MEMCPY(&command.LeWriteSuggestedDefDataLength.suggestedMaxTxOctets, &_suggestedMaxTxOctets, sizeof(UInt16));
#undef _suggestedMaxTxOctets
#define _suggestedMaxTxTime                                           pPayload[0 + 2*1]
            MEMCPY(&command.LeWriteSuggestedDefDataLength.suggestedMaxTxTime, &_suggestedMaxTxTime, sizeof(UInt16));
#undef _suggestedMaxTxTime
            function = gpBle_LeWriteSuggestedDefDataLength;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeLeAddDeviceToResolvingList:
        {
#define _peerIdentityAddressType                                      pPayload[0]
            command.LeAddDeviceToResolvingList.peerIdentityAddressType = _peerIdentityAddressType;
#undef _peerIdentityAddressType
#define _peerIdentityAddress                                          pPayload[0 + 1]
            MEMCPY(&command.LeAddDeviceToResolvingList.peerIdentityAddress, &_peerIdentityAddress, sizeof(BtDeviceAddress_t));
#undef _peerIdentityAddress
#define _peerIRK                                                      pPayload[0 + 1 + 6*1]
            MEMCPY(&command.LeAddDeviceToResolvingList.peerIRK, &_peerIRK, sizeof(gpHci_IRK_t));
#undef _peerIRK
#define _localIRK                                                     pPayload[0 + 1 + 6*1 + 1 * 16*1]
            MEMCPY(&command.LeAddDeviceToResolvingList.localIRK, &_localIRK, sizeof(gpHci_IRK_t));
#undef _localIRK
            function = gpBle_LeAddDeviceToResolvingList;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeLeRemoveDeviceFromResolvingList:
        {
#define _peerIdentityAddressType                                      pPayload[0]
            command.LeRemoveDeviceFromResolvingList.peerIdentityAddressType = _peerIdentityAddressType;
#undef _peerIdentityAddressType
#define _peerIdentityAddress                                          pPayload[0 + 1]
            MEMCPY(&command.LeRemoveDeviceFromResolvingList.peerIdentityAddress, &_peerIdentityAddress, sizeof(BtDeviceAddress_t));
#undef _peerIdentityAddress
            function = gpBle_LeRemoveDeviceFromResolvingList;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeLeClearResolvingList:
        {
            function = gpBle_LeClearResolvingList;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeLeReadResolvingListSize:
        {
            function = gpBle_LeReadResolvingListSize;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeLeSetAddressResolutionEnable:
        {
#define _enable                                                       pPayload[0]
            command.LeSetAddressResolutionEnable.enable = _enable;
#undef _enable
            function = gpBle_LeSetAddressResolutionEnable;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeLeSetResolvablePrivateAddressTimeout:
        {
#define _rpa_timeout                                                  pPayload[0]
            MEMCPY(&command.LeSetResolvablePrivateAddressTimeout.rpa_timeout, &_rpa_timeout, sizeof(UInt16));
#undef _rpa_timeout
            function = gpBle_LeSetResolvablePrivateAddressTimeout;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED)
        case gpHci_OpCodeLeReadMaxDataLength:
        {
            function = gpBle_LeReadMaxDataLength;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED)
        case gpHci_OpCodeLeReadPhy:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeReadPhy.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
            function = gpBle_LeReadPhy;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED)
        case gpHci_OpCodeLeSetDefaultPhy:
        {
#define _allPhys                                                      pPayload[0]
            command.LeSetDefaultPhy.allPhys = _allPhys;
#undef _allPhys
#define _txPhys                                                       pPayload[0 + 1]
            MEMCPY(&command.LeSetDefaultPhy.txPhys, &_txPhys, sizeof(gpHci_PhyMask_t));
#undef _txPhys
#define _rxPhys                                                       pPayload[0 + 1 + 1]
            MEMCPY(&command.LeSetDefaultPhy.rxPhys, &_rxPhys, sizeof(gpHci_PhyMask_t));
#undef _rxPhys
            function = gpBle_LeSetDefaultPhy;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED)
        case gpHci_OpCodeLeSetPhy:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeSetPhy.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _allPhys                                                      pPayload[0 + 2*1]
            command.LeSetPhy.allPhys = _allPhys;
#undef _allPhys
#define _txPhys                                                       pPayload[0 + 2*1 + 1]
            MEMCPY(&command.LeSetPhy.txPhys, &_txPhys, sizeof(gpHci_PhyMask_t));
#undef _txPhys
#define _rxPhys                                                       pPayload[0 + 2*1 + 1 + 1]
            MEMCPY(&command.LeSetPhy.rxPhys, &_rxPhys, sizeof(gpHci_PhyMask_t));
#undef _rxPhys
#define _phyOptions                                                   pPayload[0 + 2*1 + 1 + 1 + 1]
            MEMCPY(&command.LeSetPhy.phyOptions, &_phyOptions, sizeof(gpHci_PhyOptions_t));
#undef _phyOptions
            function = gpBle_LeSetPhy;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeLeEnhancedReceiverTest:
        {
#define _rxchannel                                                    pPayload[0]
            command.LeEnhancedReceiverTest.rxchannel = _rxchannel;
#undef _rxchannel
#define _phy                                                          pPayload[0 + 1]
            command.LeEnhancedReceiverTest.phy = _phy;
#undef _phy
#define _modulationIndex                                              pPayload[0 + 1 + 1]
            command.LeEnhancedReceiverTest.modulationIndex = _modulationIndex;
#undef _modulationIndex
            function = gpBle_LeEnhancedReceiverTest;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeLeEnhancedTransmitterTest:
        {
#define _txchannel                                                    pPayload[0]
            command.LeEnhancedTransmitterTest.txchannel = _txchannel;
#undef _txchannel
#define _length                                                       pPayload[0 + 1]
            command.LeEnhancedTransmitterTest.length = _length;
#undef _length
#define _payload                                                      pPayload[0 + 1 + 1]
            command.LeEnhancedTransmitterTest.payload = _payload;
#undef _payload
#define _phy                                                          pPayload[0 + 1 + 1 + 1]
            command.LeEnhancedTransmitterTest.phy = _phy;
#undef _phy
            function = gpBle_LeEnhancedTransmitterTest;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
        case gpHci_OpCodeLeReadTransmitPower:
        {
            function = gpBle_LeReadTransmitPower;
            break;
        }
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeLeSetPrivacyMode:
        {
#define _peerIdentityAddressType                                      pPayload[0]
            command.LeSetPrivacyMode.peerIdentityAddressType = _peerIdentityAddressType;
#undef _peerIdentityAddressType
#define _peerIdentityAddress                                          pPayload[0 + 1]
            MEMCPY(&command.LeSetPrivacyMode.peerIdentityAddress, &_peerIdentityAddress, sizeof(BtDeviceAddress_t));
#undef _peerIdentityAddress
#define _privacyMode                                                  pPayload[0 + 1 + 6*1]
            command.LeSetPrivacyMode.privacyMode = _privacyMode;
#undef _privacyMode
            function = gpBle_LeSetPrivacyMode;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeLeReceiverTest_v3:
        {
#define _rxchannel                                                    pPayload[0]
            command.LeReceiverTest_v3.rxchannel = _rxchannel;
#undef _rxchannel
#define _phy                                                          pPayload[0 + 1]
            command.LeReceiverTest_v3.phy = _phy;
#undef _phy
#define _modulationIndex                                              pPayload[0 + 1 + 1]
            command.LeReceiverTest_v3.modulationIndex = _modulationIndex;
#undef _modulationIndex
#define _expectedCteLengthUnit                                        pPayload[0 + 1 + 1 + 1]
            command.LeReceiverTest_v3.expectedCteLengthUnit = _expectedCteLengthUnit;
#undef _expectedCteLengthUnit
#define _expectedCteType                                              pPayload[0 + 1 + 1 + 1 + 1]
            command.LeReceiverTest_v3.expectedCteType = _expectedCteType;
#undef _expectedCteType
#define _expectedSlotDurations                                        pPayload[0 + 1 + 1 + 1 + 1 + 1]
            command.LeReceiverTest_v3.expectedSlotDurations = _expectedSlotDurations;
#undef _expectedSlotDurations
#define _switchingPatternLength                                       pPayload[0 + 1 + 1 + 1 + 1 + 1 + 1]
            command.LeReceiverTest_v3.switchingPatternLength = _switchingPatternLength;
#undef _switchingPatternLength
#define _antennaIDs                                                   (((UInt8*) (&pPayload[0 + 1 + 1 + 1 + 1 + 1 + 1 + 1])))
            command.LeReceiverTest_v3.antennaIDs = _antennaIDs;
#undef _antennaIDs
            function = gpBle_LeReceiverTest_v3;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeLeTransmitterTest_v3:
        {
#define _txchannel                                                    pPayload[0]
            command.LeTransmitterTest_v3.txchannel = _txchannel;
#undef _txchannel
#define _length                                                       pPayload[0 + 1]
            command.LeTransmitterTest_v3.length = _length;
#undef _length
#define _payload                                                      pPayload[0 + 1 + 1]
            command.LeTransmitterTest_v3.payload = _payload;
#undef _payload
#define _phy                                                          pPayload[0 + 1 + 1 + 1]
            command.LeTransmitterTest_v3.phy = _phy;
#undef _phy
#define _expectedCteLengthUnit                                        pPayload[0 + 1 + 1 + 1 + 1]
            command.LeTransmitterTest_v3.expectedCteLengthUnit = _expectedCteLengthUnit;
#undef _expectedCteLengthUnit
#define _expectedCteType                                              pPayload[0 + 1 + 1 + 1 + 1 + 1]
            command.LeTransmitterTest_v3.expectedCteType = _expectedCteType;
#undef _expectedCteType
#define _switchingPatternLength                                       pPayload[0 + 1 + 1 + 1 + 1 + 1 + 1]
            command.LeTransmitterTest_v3.switchingPatternLength = _switchingPatternLength;
#undef _switchingPatternLength
#define _antennaIDs                                                   (((UInt8*) (&pPayload[0 + 1 + 1 + 1 + 1 + 1 + 1 + 1])))
            command.LeTransmitterTest_v3.antennaIDs = _antennaIDs;
#undef _antennaIDs
            function = gpBle_LeTransmitterTest_v3;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
        case gpHci_OpCodeLeSetConnectionCteReceiveParameters:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeSetConnectionCteReceiveParameters.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _samplingEnable                                               pPayload[0 + 2*1]
            command.LeSetConnectionCteReceiveParameters.samplingEnable = _samplingEnable;
#undef _samplingEnable
#define _slotDurations                                                pPayload[0 + 2*1 + 1]
            command.LeSetConnectionCteReceiveParameters.slotDurations = _slotDurations;
#undef _slotDurations
#define _lengthOfSwitchingPattern                                     pPayload[0 + 2*1 + 1 + 1]
            command.LeSetConnectionCteReceiveParameters.lengthOfSwitchingPattern = _lengthOfSwitchingPattern;
#undef _lengthOfSwitchingPattern
#define _antennaIDs                                                   (((UInt8*) (&pPayload[0 + 2*1 + 1 + 1 + 1])))
            command.LeSetConnectionCteReceiveParameters.antennaIDs = _antennaIDs;
#undef _antennaIDs
            function = gpBle_LeSetConnectionCteReceiveParameters;
            break;
        }
#endif /* defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED) */
#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
        case gpHci_OpCodeLeSetConnectionCteTransmitParameters:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeSetConnectionCteTransmitParameters.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _CteTypes                                                     pPayload[0 + 2*1]
            command.LeSetConnectionCteTransmitParameters.CteTypes = _CteTypes;
#undef _CteTypes
#define _lengthOfSwitchingPattern                                     pPayload[0 + 2*1 + 1]
            command.LeSetConnectionCteTransmitParameters.lengthOfSwitchingPattern = _lengthOfSwitchingPattern;
#undef _lengthOfSwitchingPattern
#define _antennaIDs                                                   (((UInt8*) (&pPayload[0 + 2*1 + 1 + 1])))
            command.LeSetConnectionCteTransmitParameters.antennaIDs = _antennaIDs;
#undef _antennaIDs
            function = gpBle_LeSetConnectionCteTransmitParameters;
            break;
        }
#endif /* defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED) */
#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
        case gpHci_OpCodeLeConnectionCteRequestEnable:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeConnectionCteRequestEnable.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _Enable                                                       pPayload[0 + 2*1]
            command.LeConnectionCteRequestEnable.Enable = _Enable;
#undef _Enable
#define _cteRequestInterval                                           pPayload[0 + 2*1 + 1]
            MEMCPY(&command.LeConnectionCteRequestEnable.cteRequestInterval, &_cteRequestInterval, sizeof(UInt16));
#undef _cteRequestInterval
#define _requestedCteLength                                           pPayload[0 + 2*1 + 1 + 2*1]
            command.LeConnectionCteRequestEnable.requestedCteLength = _requestedCteLength;
#undef _requestedCteLength
#define _requestedCteType                                             pPayload[0 + 2*1 + 1 + 2*1 + 1]
            command.LeConnectionCteRequestEnable.requestedCteType = _requestedCteType;
#undef _requestedCteType
            function = gpBle_LeConnectionCteRequestEnable;
            break;
        }
#endif /* defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED) */
#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
        case gpHci_OpCodeLeConnectionCteResponseEnable:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.LeConnectionCteResponseEnable.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _Enable                                                       pPayload[0 + 2*1]
            command.LeConnectionCteResponseEnable.Enable = _Enable;
#undef _Enable
            function = gpBle_LeConnectionCteResponseEnable;
            break;
        }
#endif /* defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED) */
#if defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED)
        case gpHci_OpCodeLeReadAntennaInformation:
        {
            function = gpBle_LeReadAntennaInformation;
            break;
        }
#endif /* defined(GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED) */
        case gpHci_OpCodeVsdWriteDeviceAddress:
        {
#define _address                                                      pPayload[0]
            MEMCPY(&command.VsdWriteDeviceAddress.address, &_address, sizeof(BtDeviceAddress_t));
#undef _address
            function = gpBle_VsdWriteDeviceAddress;
            break;
        }
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeVsdGenerateAccessAddress:
        {
#define _codedPhy                                                     pPayload[0]
            command.VsdGenerateAccessAddress.codedPhy = _codedPhy;
#undef _codedPhy
            function = gpBle_VsdGenerateAccessAddress;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeSetVsdTestParams:
        {
#define _type                                                         pPayload[0]
            command.SetVsdTestParams.type = _type;
#undef _type
#define _length                                                       pPayload[0 + 1]
            MEMCPY(&command.SetVsdTestParams.length, &_length, sizeof(UInt16));
#undef _length
#define _value                                                        (((UInt8*) (&pPayload[0 + 1 + 2*1])))
            command.SetVsdTestParams.value = _value;
#undef _value
            function = gpBle_SetVsdTestParams;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
#if defined(GP_DIVERSITY_DEVELOPMENT) && defined(GP_DIVERSITY_BLE_PERIPHERAL) 
        case gpHci_OpCodeVsdSetDataPumpParameters:
        {
#define _connHandle                                                   pPayload[0]
            MEMCPY(&command.VsdSetDataPumpParameters.connHandle, &_connHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connHandle
#define _nrOfPackets                                                  pPayload[0 + 2*1]
            MEMCPY(&command.VsdSetDataPumpParameters.nrOfPackets, &_nrOfPackets, sizeof(UInt16));
#undef _nrOfPackets
#define _sizeOfPackets                                                pPayload[0 + 2*1 + 2*1]
            command.VsdSetDataPumpParameters.sizeOfPackets = _sizeOfPackets;
#undef _sizeOfPackets
#define _payloadType                                                  pPayload[0 + 2*1 + 2*1 + 1]
            command.VsdSetDataPumpParameters.payloadType = _payloadType;
#undef _payloadType
#define _payloadStartByte                                             pPayload[0 + 2*1 + 2*1 + 1 + 1]
            command.VsdSetDataPumpParameters.payloadStartByte = _payloadStartByte;
#undef _payloadStartByte
            function = gpBle_VsdSetDataPumpParameters;
            break;
        }
#endif /* defined(GP_DIVERSITY_DEVELOPMENT) && (defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_CENTRAL)) */
#if defined(GP_DIVERSITY_DEVELOPMENT) && defined(GP_DIVERSITY_BLE_PERIPHERAL) 
        case gpHci_OpCodeVsdSetDataPumpEnable:
        {
#define _enable                                                       pPayload[0]
            command.VsdSetDataPumpEnable.enable = _enable;
#undef _enable
            function = gpBle_VsdSetDataPumpEnable;
            break;
        }
#endif /* defined(GP_DIVERSITY_DEVELOPMENT) && (defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_CENTRAL)) */
#if defined(GP_DIVERSITY_DEVELOPMENT) && defined(GP_DIVERSITY_BLE_PERIPHERAL) 
        case gpHci_OpCodeVsdSetNullSinkEnable:
        {
#define _mode                                                         pPayload[0]
            command.VsdSetNullSinkEnable.mode = _mode;
#undef _mode
            function = gpBle_VsdSetNullSinkEnable;
            break;
        }
#endif /* defined(GP_DIVERSITY_DEVELOPMENT) && (defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_CENTRAL)) */
#if defined(GP_DIVERSITY_DEVELOPMENT) && defined(GP_DIVERSITY_BLE_PERIPHERAL) 
        case gpHci_OpCodeVsdSetAccessCode:
        {
#define _AccessCode                                                   pPayload[0]
            MEMCPY(&command.VsdSetAccessCode.AccessCode, &_AccessCode, sizeof(UInt32));
#undef _AccessCode
            function = gpBle_VsdSetAccessCode;
            break;
        }
#endif /* defined(GP_DIVERSITY_DEVELOPMENT) && (defined(GP_DIVERSITY_BLE_PERIPHERAL) || defined(GP_DIVERSITY_BLE_CENTRAL)) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeVsdSetTransmitPower:
        {
#define _transmitPower                                                pPayload[0]
            command.VsdSetTransmitPower.transmitPower = _transmitPower;
#undef _transmitPower
            function = gpBle_VsdSetTransmitPower;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeVsdSetSleep:
        {
#define _mode                                                         pPayload[0]
            command.VsdSetSleep.mode = _mode;
#undef _mode
#define _enable                                                       pPayload[0 + 1]
            command.VsdSetSleep.enable = _enable;
#undef _enable
            function = gpBle_VsdSetSleep;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeVsdDisableSlaveLatency:
        {
#define _connHandle                                                   pPayload[0]
            MEMCPY(&command.VsdDisableSlaveLatency.connHandle, &_connHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connHandle
#define _disabled                                                     pPayload[0 + 2*1]
            command.VsdDisableSlaveLatency.disabled = _disabled;
#undef _disabled
            function = gpBle_VsdDisableSlaveLatency;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
#if defined(GP_DIVERSITY_BLE_PERIPHERAL)
        case gpHci_OpCodeVsdEnCBByDefault:
        {
#define _enable                                                       pPayload[0]
            command.VsdEnCBByDefault.enable = _enable;
#undef _enable
            function = gpBle_VsdEnCBByDefault;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CENTRAL) || defined(GP_DIVERSITY_BLE_PERIPHERAL) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeVsdSetRpaPrand:
        {
#define _prand                                                        pPayload[0]
            MEMCPY(&command.VsdSetRpaPrand.prand, &_prand, sizeof(UInt32));
#undef _prand
#define _enableTimeout                                                pPayload[0 + 4*1]
            command.VsdSetRpaPrand.enableTimeout = _enableTimeout;
#undef _enableTimeout
            function = gpBle_VsdSetRpaPrand;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_DEVELOPMENT)
        case gpHci_OpCodeVsdGetBuildP4Changelist:
        {
            function = gpBle_VsdGetBuildP4Changelist;
            break;
        }
#endif /* defined(GP_DIVERSITY_DEVELOPMENT) */
#if defined(GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED)
        case gpHci_OpCodeVsdSetScanCoexParams:
        {
/* <CodeGenerator Placeholder> manual_gpHci_OpCodeVsdSetScanCoexParams */
            command.VsdSetScanCoexParams.scan_rx_wd.prio = pPayload[0];
            command.VsdSetScanCoexParams.scan_rx_wd.req = pPayload[1];
            command.VsdSetScanCoexParams.scan_rx_wd.grant = pPayload[2];
            command.VsdSetScanCoexParams.scan_rx.prio = pPayload[3];
            command.VsdSetScanCoexParams.scan_rx.req = pPayload[4];
            command.VsdSetScanCoexParams.scan_rx.grant = pPayload[5];
            command.VsdSetScanCoexParams.scan_tx.prio = pPayload[6];
            command.VsdSetScanCoexParams.scan_tx.req = pPayload[7];
            command.VsdSetScanCoexParams.scan_tx.grant = pPayload[8];
            function = gpBle_VsdSetScanCoexParams;
            break;
/* </CodeGenerator Placeholder> manual_gpHci_OpCodeVsdSetScanCoexParams */
        }
#endif /* defined(GP_DIVERSITY_BLE_SCAN_COEX_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeVsdGeneratePeerResolvableAddress:
        {
#define _peerIdentityAddressType                                      pPayload[0]
            command.VsdGeneratePeerResolvableAddress.peerIdentityAddressType = _peerIdentityAddressType;
#undef _peerIdentityAddressType
#define _peerIdentityAddress                                          pPayload[0 + 1]
            MEMCPY(&command.VsdGeneratePeerResolvableAddress.peerIdentityAddress, &_peerIdentityAddress, sizeof(BtDeviceAddress_t));
#undef _peerIdentityAddress
            function = gpBle_VsdGeneratePeerResolvableAddress;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeVsdGenerateLocalResolvableAddress:
        {
#define _peerIdentityAddressType                                      pPayload[0]
            command.VsdGenerateLocalResolvableAddress.peerIdentityAddressType = _peerIdentityAddressType;
#undef _peerIdentityAddressType
#define _peerIdentityAddress                                          pPayload[0 + 1]
            MEMCPY(&command.VsdGenerateLocalResolvableAddress.peerIdentityAddress, &_peerIdentityAddress, sizeof(BtDeviceAddress_t));
#undef _peerIdentityAddress
            function = gpBle_VsdGenerateLocalResolvableAddress;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX)
        case gpHci_OpCodeVsdUnsolicitedCteTxEnable:
        {
#define _connectionHandle                                             pPayload[0]
            MEMCPY(&command.VsdUnsolicitedCteTxEnable.connectionHandle, &_connectionHandle, sizeof(gpHci_ConnectionHandle_t));
#undef _connectionHandle
#define _Enable                                                       pPayload[0 + 2*1]
            command.VsdUnsolicitedCteTxEnable.Enable = _Enable;
#undef _Enable
#define _cteTxInterval                                                pPayload[0 + 2*1 + 1]
            MEMCPY(&command.VsdUnsolicitedCteTxEnable.cteTxInterval, &_cteTxInterval, sizeof(UInt16));
#undef _cteTxInterval
#define _cteLength                                                    pPayload[0 + 2*1 + 1 + 2*1]
            command.VsdUnsolicitedCteTxEnable.cteLength = _cteLength;
#undef _cteLength
#define _cteType                                                      pPayload[0 + 2*1 + 1 + 2*1 + 1]
            command.VsdUnsolicitedCteTxEnable.cteType = _cteType;
#undef _cteType
            function = gpBle_VsdUnsolicitedCteTxEnable;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_CTE_SUPPORT_UNSOLICITED_TX) */
#if defined(GP_DIVERSITY_DEVELOPMENT) && (defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED))
        case gpHci_OpCodeVsdRNG:
        {
#define _RNG_Source                                                   pPayload[0]
            command.VsdRNG.RNG_Source = _RNG_Source;
#undef _RNG_Source
#define _numRandomBytes                                               pPayload[0 + 1]
            command.VsdRNG.numRandomBytes = _numRandomBytes;
#undef _numRandomBytes
            function = gpBle_VsdRNG;
            break;
        }
#endif /* defined(GP_DIVERSITY_DEVELOPMENT) && (defined(GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED)) */
        case gpHci_OpCodeSetLeMetaVSDEvent:
        {
#define _eventCode                                                    pPayload[0]
            command.SetLeMetaVSDEvent.eventCode = _eventCode;
#undef _eventCode
            function = gpBle_SetLeMetaVSDEvent;
            break;
        }
        case gpHci_OpCodeResetLeMetaVSDEvent:
        {
#define _eventCode                                                    pPayload[0]
            command.ResetLeMetaVSDEvent.eventCode = _eventCode;
#undef _eventCode
            function = gpBle_ResetLeMetaVSDEvent;
            break;
        }
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeVsdReadResolvingListCurrentSize:
        {
            function = gpBle_VsdReadResolvingListCurrentSize;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED)
        case gpHci_OpCodeVsdSetResolvingListMaxSize:
        {
#define _maxSize                                                      pPayload[0]
            command.VsdSetResolvingListMaxSize.maxSize = _maxSize;
#undef _maxSize
            function = gpBle_VsdSetResolvingListMaxSize;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED) */
#if defined(GP_DIVERSITY_DEVELOPMENT)
        case gpHci_OpCodeVsdGetRtMgrVersion:
        {
            function = gpBle_VsdGetRtMgrVersion;
            break;
        }
#endif /* defined(GP_DIVERSITY_DEVELOPMENT) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeVsdEnhancedReceiverTest:
        {
#define _rxchannel                                                    pPayload[0]
            command.VsdEnhancedReceiverTest.rxchannel = _rxchannel;
#undef _rxchannel
#define _phyMask                                                      pPayload[0 + 1]
            MEMCPY(&command.VsdEnhancedReceiverTest.phyMask, &_phyMask, sizeof(gpHci_PhyMask_t));
#undef _phyMask
#define _modulationIndex                                              pPayload[0 + 1 + 1]
            command.VsdEnhancedReceiverTest.modulationIndex = _modulationIndex;
#undef _modulationIndex
#define _accesscode                                                   pPayload[0 + 1 + 1 + 1]
            MEMCPY(&command.VsdEnhancedReceiverTest.accesscode, &_accesscode, sizeof(UInt32));
#undef _accesscode
#define _antenna                                                      pPayload[0 + 1 + 1 + 1 + 4*1]
            command.VsdEnhancedReceiverTest.antenna = _antenna;
#undef _antenna
            function = gpBle_VsdEnhancedReceiverTest;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
#if defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED)
        case gpHci_OpCodeVsdLeReceiverTest_v3:
        {
#define _rxchannel                                                    pPayload[0]
            command.VsdLeReceiverTest_v3.rxchannel = _rxchannel;
#undef _rxchannel
#define _phyMask                                                      pPayload[0 + 1]
            MEMCPY(&command.VsdLeReceiverTest_v3.phyMask, &_phyMask, sizeof(gpHci_PhyMask_t));
#undef _phyMask
#define _modulationIndex                                              pPayload[0 + 1 + 1]
            command.VsdLeReceiverTest_v3.modulationIndex = _modulationIndex;
#undef _modulationIndex
#define _expectedCteLengthUnit                                        pPayload[0 + 1 + 1 + 1]
            command.VsdLeReceiverTest_v3.expectedCteLengthUnit = _expectedCteLengthUnit;
#undef _expectedCteLengthUnit
#define _expectedCteType                                              pPayload[0 + 1 + 1 + 1 + 1]
            command.VsdLeReceiverTest_v3.expectedCteType = _expectedCteType;
#undef _expectedCteType
#define _expectedSlotDurations                                        pPayload[0 + 1 + 1 + 1 + 1 + 1]
            command.VsdLeReceiverTest_v3.expectedSlotDurations = _expectedSlotDurations;
#undef _expectedSlotDurations
#define _switchingPatternLength                                       pPayload[0 + 1 + 1 + 1 + 1 + 1 + 1]
            command.VsdLeReceiverTest_v3.switchingPatternLength = _switchingPatternLength;
#undef _switchingPatternLength
#define _antennaIDs                                                   (((UInt8*) (&pPayload[0 + 1 + 1 + 1 + 1 + 1 + 1 + 1])))
            command.VsdLeReceiverTest_v3.antennaIDs = _antennaIDs;
#undef _antennaIDs
#define _accesscode                                                   pPayload[0 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 74]
            MEMCPY(&command.VsdLeReceiverTest_v3.accesscode, &_accesscode, sizeof(UInt32));
#undef _accesscode
#define _antenna                                                      pPayload[0 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 74 + 4*1]
            command.VsdLeReceiverTest_v3.antenna = _antenna;
#undef _antenna
            function = gpBle_VsdLeReceiverTest_v3;
            break;
        }
#endif /* defined(GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED) */
        case gpHci_OpCodeUnknownOpCode:
        {
            function = gpBle_UnknownOpCode;
            break;
        }
        default:
        {
            function = gpBle_UnknownOpCode;
            GP_LOG_SYSTEM_PRINTF("Unknown Opcode: 0x%lx",2, (long unsigned int) OpCode);
            break;
        }
    }

    gpBle_ExecuteCommand(gpBle_EventServer_Wrapper, OpCode, function, &command);
}
