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

/** @file "gpHci_clientServerCmdId.h"
 *
 *  Host Controller Interface
 *
 *  Client Server Link Command IDs
*/

#ifndef _GPHCI_CLIENTSERVERCMDID_H_
#define _GPHCI_CLIENTSERVERCMDID_H_

/*****************************************************************************
 *                    Common timeout
 *****************************************************************************/

#ifndef GPHCI_GPCOMTIMEOUT_US
#define GPHCI_GPCOMTIMEOUT_US 10000000UL //10s
#endif //GPHCI_GPCOMTIMEOUT_US

/*****************************************************************************
 *                    Component Specific Command IDs
 *****************************************************************************/

#define gpHci_Init_CmdId                                             0x01 /*01*/
#define gpHci_processCommand_CmdId                                   0x02 /*02*/
#define gpHci_processData_CmdId                                      0x03 /*03*/
#define gpHci_commandsEnabled_CmdId                                  0x06 /*06*/
#define gpHci_stopCommands_CmdId                                     0x07 /*07*/
#define gpHci_resumeCommands_CmdId                                   0x08 /*08*/
#define gpHci_StoreCrossOverCommand_CmdId                            0x87 /*135*/
#define gpHci_ExecuteCrossOverCommand_CmdId                          0x88 /*136*/
#define gpBle_Disconnect_CmdId                                       0x-1 /*-1*/
#define gpBle_ReadRemoteVersionInfo_CmdId                            0x-1 /*-1*/
#define gpBle_SetEventMask_CmdId                                     0x-1 /*-1*/
#define gpBle_Reset_CmdId                                            0x-1 /*-1*/
#define gpBle_ReadTransmitPowerLevel_CmdId                           0x-1 /*-1*/
#define gpBle_HostBufferSize_CmdId                                   0x-1 /*-1*/
#define gpBle_SetEventMaskPage2_CmdId                                0x-1 /*-1*/
#define gpBle_ReadAuthenticatedPayloadTO_CmdId                       0x-1 /*-1*/
#define gpBle_WriteAuthenticatedPayloadTO_CmdId                      0x-1 /*-1*/
#define gpBle_ReadLocalVersionInformation_CmdId                      0x-1 /*-1*/
#define gpBle_ReadLocalSupportedCommands_CmdId                       0x-1 /*-1*/
#define gpBle_ReadLocalSupportedFeatures_CmdId                       0x-1 /*-1*/
#define gpBle_ReadBufferSize_CmdId                                   0x-1 /*-1*/
#define gpBle_ReadBdAddr_CmdId                                       0x-1 /*-1*/
#define gpBle_ReadRSSI_CmdId                                         0x-1 /*-1*/
#define gpBle_LeSetEventMask_CmdId                                   0x-1 /*-1*/
#define gpBle_LeReadBufferSize_CmdId                                 0x-1 /*-1*/
#define gpBle_LeReadLocalSupportedFeatures_CmdId                     0x-1 /*-1*/
#define gpBle_LeSetRandomAddress_CmdId                               0x-1 /*-1*/
#define gpBle_LeSetAdvertisingParameters_CmdId                       0x-1 /*-1*/
#define gpBle_LeReadAdvertisingChannelTxPower_CmdId                  0x-1 /*-1*/
#define gpBle_LeSetAdvertisingData_CmdId                             0x-1 /*-1*/
#define gpBle_LeSetScanResponseData_CmdId                            0x-1 /*-1*/
#define gpBle_LeSetAdvertiseEnable_CmdId                             0x-1 /*-1*/
#define gpBle_LeSetScanParameters_CmdId                              0x-1 /*-1*/
#define gpBle_LeSetScanEnable_CmdId                                  0x-1 /*-1*/
#define gpBle_LeCreateConnection_CmdId                               0x-1 /*-1*/
#define gpBle_LeCreateConnectionCancel_CmdId                         0x-1 /*-1*/
#define gpBle_LeReadWhiteListSize_CmdId                              0x-1 /*-1*/
#define gpBle_LeClearWhiteList_CmdId                                 0x-1 /*-1*/
#define gpBle_LeAddDeviceToWhiteList_CmdId                           0x-1 /*-1*/
#define gpBle_LeRemoveDeviceFromWhiteList_CmdId                      0x-1 /*-1*/
#define gpBle_LeConnectionUpdate_CmdId                               0x-1 /*-1*/
#define gpBle_LeSetHostChannelClassification_CmdId                   0x-1 /*-1*/
#define gpBle_LeReadChannelMap_CmdId                                 0x-1 /*-1*/
#define gpBle_LeReadRemoteFeatures_CmdId                             0x-1 /*-1*/
#define gpBle_LeEncrypt_CmdId                                        0x-1 /*-1*/
#define gpBle_LeRand_CmdId                                           0x-1 /*-1*/
#define gpBle_LeStartEncryption_CmdId                                0x-1 /*-1*/
#define gpBle_LeLongTermKeyRequestReply_CmdId                        0x-1 /*-1*/
#define gpBle_LeLongTermKeyRequestNegativeReply_CmdId                0x-1 /*-1*/
#define gpBle_LeReadSupportedStates_CmdId                            0x-1 /*-1*/
#define gpBle_LeReceiverTest_CmdId                                   0x-1 /*-1*/
#define gpBle_LeTransmitterTest_CmdId                                0x-1 /*-1*/
#define gpBle_LeTestEnd_CmdId                                        0x-1 /*-1*/
#define gpBle_LeRemoteConnectionParamRequestReply_CmdId              0x-1 /*-1*/
#define gpBle_LeRemoteConnectionParamRequestNegReply_CmdId           0x-1 /*-1*/
#define gpBle_LeSetDataLength_CmdId                                  0x-1 /*-1*/
#define gpBle_LeReadSuggestedDefDataLength_CmdId                     0x-1 /*-1*/
#define gpBle_LeWriteSuggestedDefDataLength_CmdId                    0x-1 /*-1*/
#define gpBle_LeAddDeviceToResolvingList_CmdId                       0x-1 /*-1*/
#define gpBle_LeRemoveDeviceFromResolvingList_CmdId                  0x-1 /*-1*/
#define gpBle_LeClearResolvingList_CmdId                             0x-1 /*-1*/
#define gpBle_LeReadResolvingListSize_CmdId                          0x-1 /*-1*/
#define gpBle_LeSetAddressResolutionEnable_CmdId                     0x-1 /*-1*/
#define gpBle_LeSetResolvablePrivateAddressTimeout_CmdId             0x-1 /*-1*/
#define gpBle_LeReadMaxDataLength_CmdId                              0x-1 /*-1*/
#define gpBle_LeReadPhy_CmdId                                        0x-1 /*-1*/
#define gpBle_LeSetDefaultPhy_CmdId                                  0x-1 /*-1*/
#define gpBle_LeSetPhy_CmdId                                         0x-1 /*-1*/
#define gpBle_LeEnhancedReceiverTest_CmdId                           0x-1 /*-1*/
#define gpBle_LeEnhancedTransmitterTest_CmdId                        0x-1 /*-1*/
#define gpBle_LeSetAdvertisingSetRandomAddress_CmdId                 0x-1 /*-1*/
#define gpBle_LeSetExtendedAdvertisingParameters_CmdId               0x-1 /*-1*/
#define gpBle_LeSetExtendedAdvertisingData_CmdId                     0x-1 /*-1*/
#define gpBle_LeSetExtendedScanResponseData_CmdId                    0x-1 /*-1*/
#define gpBle_LeSetExtendedAdvertisingEnable_CmdId                   0x-1 /*-1*/
#define gpBle_LeReadMaximumAdvertisingDataLength_CmdId               0x-1 /*-1*/
#define gpBle_LeReadNumberSupportedAdvertisingSets_CmdId             0x-1 /*-1*/
#define gpBle_LeRemoveAdvertisingSet_CmdId                           0x-1 /*-1*/
#define gpBle_LeClearAdvertisingSets_CmdId                           0x-1 /*-1*/
#define gpBle_LeSetPeriodicAdvertisingParameters_CmdId               0x-1 /*-1*/
#define gpBle_LeSetPeriodicAdvertisingData_CmdId                     0x-1 /*-1*/
#define gpBle_LeSetPeriodicAdvertisingEnable_CmdId                   0x-1 /*-1*/
#define gpBle_LeSetExtendedScanParameters_CmdId                      0x-1 /*-1*/
#define gpBle_LeSetExtendedScanEnable_CmdId                          0x-1 /*-1*/
#define gpBle_LeExtendedCreateConnection_CmdId                       0x-1 /*-1*/
#define gpBle_LePeriodicAdvertisingCreateSync_CmdId                  0x-1 /*-1*/
#define gpBle_LePeriodicAdvertisingCreateSyncCancel_CmdId            0x-1 /*-1*/
#define gpBle_LePeriodicAdvertisingTerminateSync_CmdId               0x-1 /*-1*/
#define gpBle_LeAddDeviceToPeriodicAdvertiserList_CmdId              0x-1 /*-1*/
#define gpBle_LeRemoveDeviceFromPeriodicAdvertiserList_CmdId         0x-1 /*-1*/
#define gpBle_LeClearPeriodicAdvertiserList_CmdId                    0x-1 /*-1*/
#define gpBle_LeReadPeriodicAdvertiserListSize_CmdId                 0x-1 /*-1*/
#define gpBle_LeReadTransmitPower_CmdId                              0x-1 /*-1*/
#define gpBle_LeReadRfPathCompensation_CmdId                         0x-1 /*-1*/
#define gpBle_LeWriteRfPathCompensation_CmdId                        0x-1 /*-1*/
#define gpBle_LeSetPrivacyMode_CmdId                                 0x-1 /*-1*/
#define gpBle_LeReceiverTest_v3_CmdId                                0x-1 /*-1*/
#define gpBle_LeTransmitterTest_v3_CmdId                             0x-1 /*-1*/
#define gpBle_LeSetConnectionlessCteTransmitParameters_CmdId         0x-1 /*-1*/
#define gpBle_LeSetConnectionlessCteTransmitEnable_CmdId             0x-1 /*-1*/
#define gpBle_LeSetConnectionCteReceiveParameters_CmdId              0x-1 /*-1*/
#define gpBle_LeSetConnectionCteTransmitParameters_CmdId             0x-1 /*-1*/
#define gpBle_LeConnectionCteRequestEnable_CmdId                     0x-1 /*-1*/
#define gpBle_LeConnectionCteResponseEnable_CmdId                    0x-1 /*-1*/
#define gpBle_LeReadAntennaInformation_CmdId                         0x-1 /*-1*/
#define gpBle_LeSetPeriodicAdvertisingReceiveEnable_CmdId            0x-1 /*-1*/
#define gpBle_LePeriodicAdvertisingSyncTransfer_CmdId                0x-1 /*-1*/
#define gpBle_LePeriodicAdvertisingSetInfoTransfer_CmdId             0x-1 /*-1*/
#define gpBle_LeSetPeriodicAdvertisingSyncTransferParameters_CmdId   0x-1 /*-1*/
#define gpBle_LeSetDefaultPeriodicAdvertisingSyncTransferParameters_CmdId 0x-1 /*-1*/
#define gpBle_VsdWriteDeviceAddress_CmdId                            0x-1 /*-1*/
#define gpBle_VsdGenerateAccessAddress_CmdId                         0x-1 /*-1*/
#define gpBle_SetVsdTestParams_CmdId                                 0x-1 /*-1*/
#define gpBle_VsdSetDataPumpParameters_CmdId                         0x-1 /*-1*/
#define gpBle_VsdSetDataPumpEnable_CmdId                             0x-1 /*-1*/
#define gpBle_VsdSetNullSinkEnable_CmdId                             0x-1 /*-1*/
#define gpBle_VsdSetAccessCode_CmdId                                 0x-1 /*-1*/
#define gpBle_VsdToggleSNandNESN_CmdId                               0x-1 /*-1*/
#define gpBle_VsdSetAccessCodeValidationParameters_CmdId             0x-1 /*-1*/
#define gpBle_VsdSetTransmitPower_CmdId                              0x-1 /*-1*/
#define gpBle_VsdSetSleep_CmdId                                      0x-1 /*-1*/
#define gpBle_VsdDisableSlaveLatency_CmdId                           0x-1 /*-1*/
#define gpBle_VsdEnCBByDefault_CmdId                                 0x-1 /*-1*/
#define gpBle_VsdSetRpaPrand_CmdId                                   0x-1 /*-1*/
#define gpBle_VsdGetBuildP4Changelist_CmdId                          0x-1 /*-1*/
#define gpBle_VsdSetExtAdvLRPhy_CmdId                                0x-1 /*-1*/
#define gpBle_VsdSetAdvertisingCoexParams_CmdId                      0x-1 /*-1*/
#define gpBle_VsdSetScanCoexParams_CmdId                             0x-1 /*-1*/
#define gpBle_VsdSetInitCoexParams_CmdId                             0x-1 /*-1*/
#define gpBle_VsdGeneratePeerResolvableAddress_CmdId                 0x-1 /*-1*/
#define gpBle_VsdGenerateLocalResolvableAddress_CmdId                0x-1 /*-1*/
#define gpBle_VsdSetMinUsedChannels_CmdId                            0x-1 /*-1*/
#define gpBle_VsdSetConnSlaCoexParams_CmdId                          0x-1 /*-1*/
#define gpBle_VsdSetConnMasCoexParams_CmdId                          0x-1 /*-1*/
#define gpBle_VsdUnsolicitedCteTxEnable_CmdId                        0x-1 /*-1*/
#define gpBle_VsdRNG_CmdId                                           0x-1 /*-1*/
#define gpBle_SetLeMetaVSDEvent_CmdId                                0x-1 /*-1*/
#define gpBle_ResetLeMetaVSDEvent_CmdId                              0x-1 /*-1*/
#define gpBle_VsdBleCoexAlternatePriority_CmdId                      0x-1 /*-1*/
#define gpBle_VsdReadResolvingListCurrentSize_CmdId                  0x-1 /*-1*/
#define gpBle_VsdSetResolvingListMaxSize_CmdId                       0x-1 /*-1*/
#define gpBle_VsdSetConnSlaCoexUpdateParams_CmdId                    0x-1 /*-1*/
#define gpBle_VsdSetConnMasCoexUpdateParams_CmdId                    0x-1 /*-1*/
#define gpBle_VsdSetExponentialBase_CmdId                            0x-1 /*-1*/
#define gpBle_VsdSetMinimalSubeventDistance_CmdId                    0x-1 /*-1*/
#define gpBle_UnknownOpCode_CmdId                                    0x-1 /*-1*/
#define gpHci_ConnectionCompleteEvent_CmdId                          0xcb /*203*/
#define gpHci_DisconnectionCompleteEvent_CmdId                       0x05 /*05*/
#define gpHci_EncryptionChangeEvent_CmdId                            0x08 /*08*/
#define gpHci_ReadRemoteVersionCompleteEvent_CmdId                   0x0c /*12*/
#define gpHci_CommandCompleteEvent_CmdId                             0x0e /*14*/
#define gpHci_CommandStatusEvent_CmdId                               0x0f /*15*/
#define gpHci_HardwareErrorEvent_CmdId                               0x10 /*16*/
#define gpHci_NumberOfCompletedPacketsEvent_CmdId                    0x13 /*19*/
#define gpHci_DataBufferOverflowEvent_CmdId                          0x1a /*26*/
#define gpHci_EncryptionKeyRefreshCompleteEvent_CmdId                0x30 /*48*/
#define gpHci_LEMetaEvent_CmdId                                      0x3e /*62*/
#define gpHci_AuthenticationPayloadTOEvent_CmdId                     0x57 /*87*/
#define gpHci_SendHciDataFrameToHost_CmdId                           0x58 /*88*/
#define gpHci_VsdSinkRxIndication_CmdId                              0x59 /*89*/
#define gpHci_VsdEvent_CmdId                                         0xfd /*253*/

/*****************************************************************************
 *                    Fixed Command IDs
 *****************************************************************************/

#define gpHci_Acknowledge_CmdId                                   0xfe /*254*/
#define gpHci_GetServerCompatibilityNumber_CmdId                  0xfd /*253*/
/*****************************************************************************
 *                    Acknowledge command Results
 *****************************************************************************/

/** @enum gpHci_AckStatus_t */
//@{
/** @brief Command processed correctly */
#define gpHci_AckStatusSuccess                                       0x0
/** @brief Command not known by device */
#define gpHci_AckStatusUnknownCommand                                0x1
/** @brief Command unsupported */
#define gpHci_AckStatusUnsupportedCommand                            0x2
/** @brief Command parameter length incorrect */
#define gpHci_AckStatusWrongParameterLength                          0x3
/** @brief Command execution failure */
#define gpHci_AckStatusExecutionFailed                               0x4
/** @brief Device not able to process command at this time */
#define gpHci_AckStatusBusy                                          0x5
/** @brief The connected PC client is not the expected client. Last connected client takes ownership of the board, rendering previously connected client invalid */
#define gpHci_AckStatusClientIDMismatch                              0x6
/** @typedef AckStatus_t
    @brief Serial status reported on communication
*/
typedef UInt8                             gpHci_AckStatus_t;
//@}

#endif //_GPHCI_CLIENTSERVERCMDID_H_

