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
 * $Header: //depot/release/Embedded/Components/Qorvo/BleController/v2.10.2.0/comps/gpBleConfig/src/gpBleConfig.c#1 $
 * $Change: 187624 $
 * $DateTime: 2021/12/20 10:58:50 $
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

//#define GP_LOCAL_LOG

#define GP_COMPONENT_ID GP_COMPONENT_ID_BLECONFIG

#include "gpBle.h"
#include "gpBleComps.h"
#include "gpBle_defs.h"
#include "gpAssert.h"
#include "gpLog.h"
#include "gpHal.h"
#include "gpBleConfig.h"

#if defined(GP_DIVERSITY_BLE_BROADCASTER) || defined(GP_DIVERSITY_BLE_SLAVE) || defined(GP_DIVERSITY_BLE_ADVERTISER)
#include "gpBleAdvertiser.h"
#endif //GP_DIVERSITY_BLE_BROADCASTER || GP_DIVERSITY_BLE_SLAVE || GP_DIVERSITY_BLE_ADVERTISER

#if defined(GP_DIVERSITY_BLE_OBSERVER) || defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SCANNER)
#include "gpBleScanner.h"
#endif //GP_DIVERSITY_BLE_OBSERVER || GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SCANNER

#if defined(GP_DIVERSITY_BLE_MASTER) || defined(GP_DIVERSITY_BLE_SLAVE)
#include "gpBleLlcp.h"
#include "gpBleInitiator.h"
#include "gpBleLlcpProcedures.h"
#endif //GP_DIVERSITY_BLE_MASTER || GP_DIVERSITY_BLE_SLAVE
#ifdef GP_COMP_RTIL
#include "gpRtIl.h"
#endif //GP_COMP_RTIL
/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

// Current max (spec allows up to 42 (0 till 41 incl), currently limited to 41 as no more bytes are in use)
#define BLECONFIG_SUPPORTED_COMMANDS_LENGTH         42

// All command masks (within the octet)

// Octet 0
#define BLE_CMD_DISCONNECT                      (1 << 5)
// Octet 2
#define BLE_CMD_READ_REM_VERSION                (1 << 7)
// Octet 5
#define BLE_CMD_SET_EVENT_MASK                  (1 << 6)
#define BLE_CMD_RESET                           (1 << 7)
// Octet 10
#define BLE_CMD_READ_TX_POWER_LEVEL             (1 << 2)
#define BLE_CMD_SET_CONT_TO_HOST_FLOW_CTRL      (1 << 5)
#define BLE_CMD_HOST_NR_COMPLETED_PACKETS       (1 << 7)
// Octet 14
#define BLE_CMD_READ_LOCAL_VERSION_INFO         (1 << 3)
#define BLE_CMD_READ_LOCAL_SUPP_FEATURES        (1 << 5)
// Octet 15
#define BLE_CMD_READ_BD_ADDRESS                 (1 << 1)
#define BLE_CMD_READ_RSSI                       (1 << 5)
// Octet 22
#define BLE_CMD_SET_EVENT_MASK_PAGE2            (1 << 2)
// Octet 25
#define BLE_CMD_LE_SET_EVENT_MASK               (1 << 0)
#define BLE_CMD_LE_READ_BUFFER_SIZE             (1 << 1)
#define BLE_CMD_LE_READ_LOCAL_SUPP_FEATURES     (1 << 2)
#define BLE_CMD_LE_SET_RANDOM_ADDRESS           (1 << 4)
#define BLE_CMD_LE_SET_ADV_PARAMS               (1 << 5)
#define BLE_CMD_READ_ADV_CHANNEL_TX_POWER       (1 << 6)
#define BLE_CMD_LE_SET_ADV_DATA                 (1 << 7)
// Octet 26
#define BLE_CMD_LE_SET_SCAN_RSP_DATA            (1 << 0)
#define BLE_CMD_LE_SET_ADV_ENABLE               (1 << 1)
#define BLE_CMD_LE_SET_SCAN_PARAMS              (1 << 2)
#define BLE_CMD_LE_SET_SCAN_ENABLE              (1 << 3)
#define BLE_CMD_LE_CREATE_CONNECTION            (1 << 4)
#define BLE_CMD_LE_CREATE_CONNECTION_CANCEL     (1 << 5)
#define BLE_CMD_LE_READ_WHITE_LIST_SIZE         (1 << 6)
#define BLE_CMD_LE_CLEAR_WHITE_LIST             (1 << 7)
// Octet 27
#define BLE_CMD_LE_ADD_DEV_TO_WHITE_LIST        (1 << 0)
#define BLE_CMD_LE_REMOVE_DEV_FROM_WHITE_LIST   (1 << 1)
#define BLE_CMD_LE_CONNECTION_UPDATE            (1 << 2)
#define BLE_CMD_LE_SET_HOST_CHAN_CLASSIFICATION (1 << 3)
#define BLE_CMD_LE_READ_CHANNEL_MAP             (1 << 4)
#define BLE_CMD_LE_READ_REMOTE_FEATURES         (1 << 5)
#define BLE_CMD_LE_ENCRYPT                      (1 << 6)
#define BLE_CMD_LE_RAND                         (1 << 7)
// Octet 28
#define BLE_CMD_LE_START_ENCRYPTION             (1 << 0)
#define BLE_CMD_LE_LTK_REQUEST_REPLY            (1 << 1)
#define BLE_CMD_LE_LTK_REQUEST_NEG_REPLY        (1 << 2)
#define BLE_CMD_LE_READ_SUPPORTED_STATES        (1 << 3)
#define BLE_CMD_LE_RECEIVER_TEST                (1 << 4)
#define BLE_CMD_LE_TRANSMITTER_TEST             (1 << 5)
#define BLE_CMD_LE_TEST_END                     (1 << 6)
// Octet 32
#define BLE_CMD_READ_AUTH_PAYLOAD_TO            (1 << 4)
#define BLE_CMD_WRITE_AUTH_PAYLOAD_TO           (1 << 5)
// Octet 33
#define BLE_CMD_LE_REM_CONN_PARAM_REQ_REPLY     (1 << 4)
#define BLE_CMD_LE_REM_CONN_PARAM_REQ_NEG_REPLY (1 << 5)
#define BLE_CMD_LE_SET_DATA_LENGTH              (1 << 6)
#define BLE_CMD_LE_READ_SUG_DEF_DATA_LENGTH     (1 << 7)
// Octet 34
#define BLE_CMD_LE_WRITE_SUG_DEF_DATA_LENGTH    (1 << 0)
#define BLE_CMD_LE_ADD_DEV_TO_RESOL_LIST        (1 << 3)
#define BLE_CMD_LE_REMOVE_DEV_FROM_RESOL_LIST   (1 << 4)
#define BLE_CMD_LE_CLEAR_RESOL_LIST             (1 << 5)
#define BLE_CMD_LE_READ_RESOL_LIST_SIZE         (1 << 6)
// Octet 35
#define BLE_CMD_LE_SET_ADDRESS_RESOLUTION_ENA   (1 << 1)
#define BLE_CMD_LE_SE_RPA_TIMEOUT               (1 << 2)
#define BLE_CMD_LE_READ_MAX_DATA_LENGTH         (1 << 3)
#define BLE_CMD_LE_READ_PHY                     (1 << 4)
#define BLE_CMD_LE_SET_DEFAULT_PHY              (1 << 5)
#define BLE_CMD_LE_SET_PHY                      (1 << 6)
#define BLE_CMD_LE_ENHANCED_RECEIVER_TEST       (1 << 7)

// Octet 36
#define BLE_CMD_LE_ENHANCED_TRANSMITTER_TEST                    (1 << 0)
#define BLE_CMD_LE__SET_ADVERTISING_SET_RANDOM_ADDRESS          (1 << 1)
#define BLE_CMD_LE_SET_EXTENDED_ADVERTISING_PARAMETERS          (1 << 2)
#define BLE_CMD_LE_SET_EXTENDED_ADVERTISING_DATA                (1 << 3)
#define BLE_CMD_LE_SET_EXTENDED_SCAN_RESPONSE_DATA              (1 << 4)
#define BLE_CMD_LE_SET_EXTENDED_ADVERTISING_ENABLE              (1 << 5)
#define BLE_CMD_LE_READ_MAXIMUM_ADVERTISING_DATA_LENGTH         (1 << 6)
#define BLE_CMD_LE_READ_NUMBER_OF_SUPPORTED_ADVERTISING_SETS    (1 << 7)

// Octet 37
#define BLE_CMD_LE_REMOVE_ADVERTISING_SET                       (1 << 0)
#define BLE_CMD_LE_CLEAR_ADVERTISING_SETS                       (1 << 1)
#define BLE_CMD_LE_SET_PERIODIC_ADVERTISING_PARAMETERS          (1 << 2)
#define BLE_CMD_LE_SET_PERIODIC_ADVERTISING_DATA                (1 << 3)
#define BLE_CMD_LE_SET_PERIODIC_ADVERTISING_ENABLE              (1 << 4)
#define BLE_CMD_LE_SET_EXTENDED_SCAN_PARAMETERS                 (1 << 5)
#define BLE_CMD_LE_SET_EXTENDED_SCAN_ENABLE                     (1 << 6)
#define BLE_CMD_LE_EXTENDED_CREATE_CONNECTION                   (1 << 7)

// Octet 38
#define BLE_CMD_LE_PERIODIC_ADVERTISING_CREATE_SYNC             (1 << 0)
#define BLE_CMD_LE_PERIODIC_ADVERTISING_CREATE_SYNC_CANCEL      (1 << 1)
#define BLE_CMD_LE_PERIODIC_ADVERTISING_TERMINATE_SYNC          (1 << 2)
#define BLE_CMD_LE_ADD_DEVICE_TO_PERIODIC_ADVERTISER_LIST       (1 << 3)
#define BLE_CMD_LE_REMOVE_DEVICE_FROM_PERIODIC_ADVERTISER_LIST  (1 << 4)
#define BLE_CMD_LE_CLEAR_PERIODIC_ADVERTISER_LIST               (1 << 5)
#define BLE_CMD_LE_READ_PERIODIC_ADVERTISER_LIST_SIZE           (1 << 6)
#define BLE_CMD_LE_READ_TRANSMIT_POWER                          (1 << 7)

// Octet 39
#define BLE_CMD_LE_READ_RF_PATH_COMPENSATION                    (1 << 0)
#define BLE_CMD_LE_WRITE_RF_PATH_COMPENSATION                   (1 << 1)
#define BLE_CMD_LE_SET_PRIVACY_MODE                             (1 << 2)
#define BLE_CMD_LE_RECEIVER_TEST_V3                             (1 << 3)
#define BLE_CMD_LE_TRANSMITTER_TEST_V3                          (1 << 4)
#define BLE_CMD_LE_SET_CONNECTIONLESS_CTE_TRANSMIT_PARAMETERS   (1 << 5)
#define BLE_CMD_LE_SET_CONNECTIONLESS_CTE_TRANSMIT_ENABLE       (1 << 6)
#define BLE_CMD_LE_SET_CONNECTIONLESS_IQ_SAMPLING_ENABLE        (1 << 7)

// Octet 40
#define BLE_CMD_LE_SET_CONNECTION_CTE_RECEIVE_PARAMETERS        (1 << 0)
#define BLE_CMD_LE_SET_CONNECTION_CTE_TRANSMIT_PARAMETERS       (1 << 1)
#define BLE_CMD_LE_CONNECTION_CTE_REQUEST_ENABLE                (1 << 2)
#define BLE_CMD_LE_CONNECTION_CTE_RESPONSE_ENABLE               (1 << 3)
#define BLE_CMD_LE_READ_ANTENNA_INFORMATION                     (1 << 4)
#define BLE_CMD_LE_SET_PERIODIC_ADVERTISING_RECEIVE_ENABLE      (1 << 5)
#define BLE_CMD_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER           (1 << 6)
#define BLE_CMD_LE_PERIODIC_ADVERTISING_SET_INFO_TRANSFER       (1 << 7)

// Octet 41
#define BLE_CMD_LE_SET_PAST_PARAMETERS                          (1 << 0)
#define BLE_CMD_LE_SET_DEFAULT_PAST_PARAMETERS                  (1 << 1)

/*************************************************************************/
// All supported command octets
#define SUPPORTED_COMMANDS_OCTET_00     (BLE_CMD_DISCONNECT)
#define SUPPORTED_COMMANDS_OCTET_01     (0x00)
#define SUPPORTED_COMMANDS_OCTET_02     (BLE_CMD_READ_REM_VERSION)
#define SUPPORTED_COMMANDS_OCTET_03     (0x00)
#define SUPPORTED_COMMANDS_OCTET_04     (0x00)
#define SUPPORTED_COMMANDS_OCTET_05     (BLE_CMD_SET_EVENT_MASK | BLE_CMD_RESET)
#define SUPPORTED_COMMANDS_OCTET_06     (0x00)
#define SUPPORTED_COMMANDS_OCTET_07     (0x00)
#define SUPPORTED_COMMANDS_OCTET_08     (0x00)
#define SUPPORTED_COMMANDS_OCTET_09     (0x00)
#define SUPPORTED_COMMANDS_OCTET_10     (BLE_CMD_READ_TX_POWER_LEVEL)
#define SUPPORTED_COMMANDS_OCTET_11     (0x00)
#define SUPPORTED_COMMANDS_OCTET_12     (0x00)
#define SUPPORTED_COMMANDS_OCTET_13     (0x00)
#define SUPPORTED_COMMANDS_OCTET_14     (BLE_CMD_READ_LOCAL_VERSION_INFO | BLE_CMD_READ_LOCAL_SUPP_FEATURES)
#define SUPPORTED_COMMANDS_OCTET_15     (BLE_CMD_READ_BD_ADDRESS | BLE_CMD_READ_RSSI)
#define SUPPORTED_COMMANDS_OCTET_16     (0x00)
#define SUPPORTED_COMMANDS_OCTET_17     (0x00)
#define SUPPORTED_COMMANDS_OCTET_18     (0x00)
#define SUPPORTED_COMMANDS_OCTET_19     (0x00)
#define SUPPORTED_COMMANDS_OCTET_20     (0x00)
#define SUPPORTED_COMMANDS_OCTET_21     (0x00)
#define SUPPORTED_COMMANDS_OCTET_22     (BLE_CMD_SET_EVENT_MASK_PAGE2)
#define SUPPORTED_COMMANDS_OCTET_23     (0x00)
#define SUPPORTED_COMMANDS_OCTET_24     (0x00)
#define SUPPORTED_COMMANDS_OCTET_25     (BLE_CMD_LE_SET_EVENT_MASK | BLE_CMD_LE_READ_BUFFER_SIZE | BLE_CMD_LE_READ_LOCAL_SUPP_FEATURES | BLE_CMD_LE_SET_RANDOM_ADDRESS | \
                                         BLE_CMD_LE_SET_ADV_PARAMS | BLE_CMD_READ_ADV_CHANNEL_TX_POWER | BLE_CMD_LE_SET_ADV_DATA)
#define SUPPORTED_COMMANDS_OCTET_26     (BLE_CMD_LE_SET_SCAN_RSP_DATA | BLE_CMD_LE_SET_ADV_ENABLE | BLE_CMD_LE_SET_SCAN_PARAMS | BLE_CMD_LE_SET_SCAN_ENABLE | \
                                         BLE_CMD_LE_CREATE_CONNECTION | BLE_CMD_LE_CREATE_CONNECTION_CANCEL | BLE_CMD_LE_READ_WHITE_LIST_SIZE | BLE_CMD_LE_CLEAR_WHITE_LIST)
#define SUPPORTED_COMMANDS_OCTET_27     (BLE_CMD_LE_ADD_DEV_TO_WHITE_LIST | BLE_CMD_LE_REMOVE_DEV_FROM_WHITE_LIST | BLE_CMD_LE_CONNECTION_UPDATE | \
                                         BLE_CMD_LE_SET_HOST_CHAN_CLASSIFICATION | BLE_CMD_LE_READ_CHANNEL_MAP | BLE_CMD_LE_READ_REMOTE_FEATURES | BLE_CMD_LE_ENCRYPT | \
                                         BLE_CMD_LE_RAND)
#define SUPPORTED_COMMANDS_OCTET_28     (BLE_CMD_LE_START_ENCRYPTION | BLE_CMD_LE_LTK_REQUEST_REPLY | BLE_CMD_LE_LTK_REQUEST_NEG_REPLY | \
                                         BLE_CMD_LE_READ_SUPPORTED_STATES | BLE_CMD_LE_RECEIVER_TEST | BLE_CMD_LE_TRANSMITTER_TEST | BLE_CMD_LE_TEST_END)
#define SUPPORTED_COMMANDS_OCTET_29     (0x00)
#define SUPPORTED_COMMANDS_OCTET_30     (0x00)
#define SUPPORTED_COMMANDS_OCTET_31     (0x00)
#define SUPPORTED_COMMANDS_OCTET_32     (BLE_CMD_READ_AUTH_PAYLOAD_TO | BLE_CMD_WRITE_AUTH_PAYLOAD_TO)
#define SUPPORTED_COMMANDS_OCTET_33     (BLE_CMD_LE_REM_CONN_PARAM_REQ_REPLY | BLE_CMD_LE_REM_CONN_PARAM_REQ_NEG_REPLY | BLE_CMD_LE_SET_DATA_LENGTH | \
                                         BLE_CMD_LE_READ_SUG_DEF_DATA_LENGTH)

#define SUPPORTED_COMMANDS_OCTET_34     (BLE_CMD_LE_WRITE_SUG_DEF_DATA_LENGTH)
#define SUPPORTED_COMMANDS_OCTET_35     (BLE_CMD_LE_READ_MAX_DATA_LENGTH | BLE_CMD_LE_READ_PHY | BLE_CMD_LE_SET_DEFAULT_PHY | BLE_CMD_LE_SET_PHY | \
                                         BLE_CMD_LE_ENHANCED_RECEIVER_TEST)

// SUPPORTED_COMMANDS_OCTET_36
#define EXTENDED_ADVERTISING_HCI_COMMANDS_OCTET36 ( 0 )
#define SUPPORTED_COMMANDS_OCTET_36     (BLE_CMD_LE_ENHANCED_TRANSMITTER_TEST | EXTENDED_ADVERTISING_HCI_COMMANDS_OCTET36)

// SUPPORTED_COMMANDS_OCTET_37
#define EXTENDED_ADVERTISING_HCI_COMMANDS_OCTET37 ( 0 )
#define EXTENDED_SCANNING_HCI_COMMANDS_OCTET37    ( 0 )
#define EXTENDED_INITIATING_HCI_COMMANDS_OCTET37  ( 0 )
#define SUPPORTED_COMMANDS_OCTET_37     (EXTENDED_ADVERTISING_HCI_COMMANDS_OCTET37 | EXTENDED_SCANNING_HCI_COMMANDS_OCTET37 | EXTENDED_INITIATING_HCI_COMMANDS_OCTET37)

// SUPPORTED_COMMANDS_OCTET_38
#ifdef GP_DIVERSITY_PERIODIC_ADVERTISING_SYNC
#define SUPPORTED_COMMANDS_OCTET_38  (   BLE_CMD_LE_READ_TRANSMIT_POWER                         \
                                       | BLE_CMD_LE_PERIODIC_ADVERTISING_CREATE_SYNC            \
                                       | BLE_CMD_LE_PERIODIC_ADVERTISING_CREATE_SYNC_CANCEL     \
                                       | BLE_CMD_LE_PERIODIC_ADVERTISING_TERMINATE_SYNC         \
                                       | BLE_CMD_LE_ADD_DEVICE_TO_PERIODIC_ADVERTISER_LIST      \
                                       | BLE_CMD_LE_REMOVE_DEVICE_FROM_PERIODIC_ADVERTISER_LIST \
                                       | BLE_CMD_LE_CLEAR_PERIODIC_ADVERTISER_LIST              \
                                       | BLE_CMD_LE_READ_PERIODIC_ADVERTISER_LIST_SIZE          \
                                     )
#else
#define SUPPORTED_COMMANDS_OCTET_38  ( BLE_CMD_LE_READ_TRANSMIT_POWER )
#endif /* GP_DIVERSITY_PERIODIC_ADVERTISING_SYNC */

// SUPPORTED_COMMANDS_OCTET_39
#define EXTENDED_ADVERTISING_HCI_COMMANDS_OCTET39 ( 0 )
#define EXTENDED_LL_PRIVACY_HCI_COMMANDS_OCTET39  ( 0 )
#define SUPPORTED_COMMANDS_OCTET_39     (BLE_CMD_LE_RECEIVER_TEST_V3 | BLE_CMD_LE_TRANSMITTER_TEST_V3 | EXTENDED_ADVERTISING_HCI_COMMANDS_OCTET39 | EXTENDED_LL_PRIVACY_HCI_COMMANDS_OCTET39)


// SUPPORTED_COMMANDS_OCTET_40
#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
#define SUPPORTED_COMMANDS_OCTET_40_DIRECTION_FINDING (BLE_CMD_LE_SET_CONNECTION_CTE_RECEIVE_PARAMETERS | \
                                                       BLE_CMD_LE_SET_CONNECTION_CTE_TRANSMIT_PARAMETERS | \
                                                       BLE_CMD_LE_CONNECTION_CTE_REQUEST_ENABLE | \
                                                       BLE_CMD_LE_CONNECTION_CTE_RESPONSE_ENABLE | \
                                                       BLE_CMD_LE_READ_ANTENNA_INFORMATION)
#else
#define SUPPORTED_COMMANDS_OCTET_40_DIRECTION_FINDING (0)
#endif //GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED

#define SUPPORTED_COMMANDS_OCTET_40_PAST_RECIPIENT (0)

#define SUPPORTED_COMMANDS_OCTET_40_PAST_SENDER (0)

#define SUPPORTED_COMMANDS_OCTET_40 (SUPPORTED_COMMANDS_OCTET_40_DIRECTION_FINDING | \
                                     SUPPORTED_COMMANDS_OCTET_40_PAST_RECIPIENT | \
                                     SUPPORTED_COMMANDS_OCTET_40_PAST_SENDER)

// SUPPORTED_COMMANDS_OCTET_41
#define SUPPORTED_COMMANDS_OCTET_41     0

// End supported command octet construction
/*************************************************************************/

#define BLE_EVENT_MASK_SIZE             8

#define BLECONFIG_RFPATHCOMP_MIN_UNITS ((Int16)-1280)
#define BLECONFIG_RFPATHCOMP_MAX_UNITS ((Int16)1280)
#define BLECONFIG_RFPATHCOMP_UNITS_TO_DB(x)  ((Int8)((x+(x>0?5:(-5)))/10))
#define BLECONFIG_RFPATHCOMP_DB_TO_UNITS(x)  (10 * x)

// Default masks for all possible event mask related commands
#define GPBLECONFIG_EVENT_MASK_DEFAULT          0x00001FFFFFFFFFFF
#define GPBLECONFIG_EVENT_MASK_PAGE2_DEFAULT    0x0000000000000000
#define GPBLECONFIG_EVENT_MASK_LE_DEFAULT       0x000000000000001F

#define GPBLECONFIG_VSD_EVENT_MASK_DEFAULT      0xFC

#define GPBLECONFIG_NR_OF_EVENTS_IN_MASK        (8 * sizeof(gpHci_EventMask_t))

#define GPBLECONFIG_CONNECTION_ACCEPT_TIMEOUT_SPEC_MIN      0x0001
#define GPBLECONFIG_CONNECTION_ACCEPT_TIMEOUT_SPEC_DEFAULT  0x1F40
#define GPBLECONFIG_CONNECTION_ACCEPT_TIMEOUT_SPEC_MAX      0xB540

// Feature masks
#define BLE_LL_FEATURE_ENCRYPTION_MASK                          BM(gpBleConfig_FeatureIdLeEncryption)
#define BLE_LL_FEATURE_CONNECTION_PARAMETERS_REQUEST_MASK       BM(gpBleConfig_FeatureIdConnectionParametersRequest)
#define BLE_LL_FEATURE_EXTENDED_REJECT_MASK                     BM(gpBleConfig_FeatureIdExtendedRejectIndication)
#define BLE_LL_FEATURE_SLAVE_FEATURE_EXCHANGE_MASK              BM(gpBleConfig_FeatureIdSlaveFeatureExchange)
#define BLE_LL_FEATURE_PING_MASK                                BM(gpBleConfig_FeatureIdLePing)
#define BLE_LL_FEATURE_DATA_PACKET_LENGTH_EXTENSION_MASK        BM(gpBleConfig_FeatureIdDataPacketLengthExtension)
#define BLE_LL_FEATURE_PRIVACY_MASK                             BM(gpBleConfig_FeatureIdLlPrivacy)
#define BLE_LL_FEATURE_EXTENDED_SCANNER_FILTER_POLICIES_MASK    BM(gpBleConfig_FeatureIdExtendedScannerFilterPolicies)
#define BLE_LL_FEATURE_2MBIT_PHY_MASK                           BM(gpBleConfig_FeatureIdLe2MbitPhy)
#define BLE_LL_FEATURE_STABLE_MODULATION_INDEX_TX_MASK          BM(gpBleConfig_FeatureIdStableModulationIndexTx)
#define BLE_LL_FEATURE_STABLE_MODULATION_INDEX_RX_MASK          BM(gpBleConfig_FeatureIdStableModulationIndexRx)
#define BLE_LL_FEATURE_CODED_PHY_MASK                           BM(gpBleConfig_FeatureIdLeCodedPhy)
#define BLE_LL_FEATURE_EXTENDED_ADVERTISING_MASK                BM(gpBleConfig_FeatureIdLeExtendedAdvertising)
#define BLE_LL_FEATURE_PERIODIC_ADVERTISING_MASK                BM(gpBleConfig_FeatureIdLePeriodicAdvertising)
#define BLE_LL_FEATURE_CHANNEL_SELECTION_ALGORITHM2_MASK        BM(gpBleConfig_FeatureIdChannelSelectionAlgorithm2)
#define BLE_LL_FEATURE_LE_POWER_CLASS1_MASK                     BM(gpBleConfig_FeatureIdLePowerClass1)
#define BLE_LL_FEATURE_MIN_NR_USED_CHANNELS_MASK                BM(gpBleConfig_FeatureIdMinNrOfUsedChannels)
#define BLE_LL_FEATURE_CONNECTION_CTE_REQ                       BM(gpBleConfig_FeatureIdConnectionCteRequest)
#define BLE_LL_FEATURE_CONNECTION_CTE_RSP                       BM(gpBleConfig_FeatureIdConnectionCteResponse)
#define BLE_LL_FEATURE_CONNECTION_CTE_TX                        BM(gpBleConfig_FeatureIdConnectionlessCteTransmitter)
#define BLE_LL_FEATURE_CONNECTION_CTE_RX                        BM(gpBleConfig_FeatureIdConnectionlessCteReceiver)
#define BLE_LL_FEATURE_ANT_SWITCHING_DURING_CTE_TX              BM(gpBleConfig_FeatureIdAntSwitchDuringCteTransmission)
#define BLE_LL_FEATURE_ANT_SWITCHING_DURING_CTE_RX              BM(gpBleConfig_FeatureIdAntSwitchDuringCteReception)
#define BLE_LL_FEATURE_RECEIVE_CTE                              BM(gpBleConfig_FeatureIdReceivingConstantToneExtension)
#define BLE_LL_FEATURE_PAST_SENDER                              BM(gpBleConfig_FeatureIdPastSender)
#define BLE_LL_FEATURE_PAST_RECIPIENT                           BM(gpBleConfig_FeatureIdPastRecipient)
#define BLE_LL_FEATURE_CIS_CENTRAL                              BM(gpBleConfig_FeatureIdCisCentral)
#define BLE_LL_FEATURE_CIS_PERIPHERAL                           BM(gpBleConfig_FeatureIdCisPeripheral)

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

typedef struct {
    gpHci_EventMask_t cbEventMask;
    gpHci_EventMask_t cbEventMaskPage2;
    gpHci_EventMask_t leEventMask;
    gpBle_HciResetCallback_t hciResetCb;
    BtDeviceAddress_t randomAddress;
    Bool              isRandomAddressSet;
    UInt16            cbVsdEventMask;
} Ble_ConfigContext_t;

/*****************************************************************************
 *                    Const Data Definitions
 *****************************************************************************/

static const UInt8 BleConfig_SupportedCommands[BLECONFIG_SUPPORTED_COMMANDS_LENGTH] =
{
    SUPPORTED_COMMANDS_OCTET_00,
    SUPPORTED_COMMANDS_OCTET_01,
    SUPPORTED_COMMANDS_OCTET_02,
    SUPPORTED_COMMANDS_OCTET_03,
    SUPPORTED_COMMANDS_OCTET_04,
    SUPPORTED_COMMANDS_OCTET_05,
    SUPPORTED_COMMANDS_OCTET_06,
    SUPPORTED_COMMANDS_OCTET_07,
    SUPPORTED_COMMANDS_OCTET_08,
    SUPPORTED_COMMANDS_OCTET_09,
    SUPPORTED_COMMANDS_OCTET_10,
    SUPPORTED_COMMANDS_OCTET_12,
    SUPPORTED_COMMANDS_OCTET_12,
    SUPPORTED_COMMANDS_OCTET_13,
    SUPPORTED_COMMANDS_OCTET_14,
    SUPPORTED_COMMANDS_OCTET_15,
    SUPPORTED_COMMANDS_OCTET_16,
    SUPPORTED_COMMANDS_OCTET_17,
    SUPPORTED_COMMANDS_OCTET_18,
    SUPPORTED_COMMANDS_OCTET_19,
    SUPPORTED_COMMANDS_OCTET_20,
    SUPPORTED_COMMANDS_OCTET_21,
    SUPPORTED_COMMANDS_OCTET_22,
    SUPPORTED_COMMANDS_OCTET_23,
    SUPPORTED_COMMANDS_OCTET_24,
    SUPPORTED_COMMANDS_OCTET_25,
    SUPPORTED_COMMANDS_OCTET_26,
    SUPPORTED_COMMANDS_OCTET_27,
    SUPPORTED_COMMANDS_OCTET_28,
    SUPPORTED_COMMANDS_OCTET_29,
    SUPPORTED_COMMANDS_OCTET_30,
    SUPPORTED_COMMANDS_OCTET_31,
    SUPPORTED_COMMANDS_OCTET_32,
    SUPPORTED_COMMANDS_OCTET_33,
    SUPPORTED_COMMANDS_OCTET_34,
    SUPPORTED_COMMANDS_OCTET_35,
    SUPPORTED_COMMANDS_OCTET_36,
    SUPPORTED_COMMANDS_OCTET_37,
    SUPPORTED_COMMANDS_OCTET_38,
    SUPPORTED_COMMANDS_OCTET_39,
    SUPPORTED_COMMANDS_OCTET_40,
    SUPPORTED_COMMANDS_OCTET_41,
};


/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

static Ble_ConfigContext_t Ble_ConfigContext;


static UInt64 Ble_LocalSupportedFeatures;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

static void BleConfig_InitLocalSupportedFeaturesMask(void);


#ifdef GP_LOCAL_LOG
static void BleConfig_DumpConfig(void);
#endif //GP_LOCAL_LOG

static void BleConfig_SetEventMask(gpHci_EventMask_t* pDst, gpHci_EventMask_t* pSrc);
static Bool BleConfig_GetRandomAddress(BtDeviceAddress_t* pAddress);

// checker and action functions
gpHci_Result_t BleConfig_SetRandomAddressChecker(void);


/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

void BleConfig_InitLocalSupportedFeaturesMask(void)
{
    Ble_LocalSupportedFeatures = 0;
















#ifdef GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED
    Ble_LocalSupportedFeatures |= BLE_LL_FEATURE_CONNECTION_CTE_RSP;
    Ble_LocalSupportedFeatures |= BLE_LL_FEATURE_ANT_SWITCHING_DURING_CTE_TX;
#endif /* GP_DIVERSITY_DIRECTIONFINDING_SUPPORTED */




}


#ifdef GP_LOCAL_LOG
void BleConfig_DumpConfig(void)
{
    gpBle_Configuration_t localConfig;

    gpBleConfig_GetConfig(&localConfig);

    GP_LOG_PRINTF("-- Ble configuration --",0);
    GP_LOG_PRINTF("LL version 0x%x LL subversion: 0x%x",0, localConfig.lmppalVersion, localConfig.lmppalSubversion);
    GP_LOG_PRINTF("HCI version 0x%x HCI revision: 0x%x",0, localConfig.hciVersion, localConfig.hciRevision);
    GP_LOG_PRINTF("Company id: 0x%x",0, localConfig.companyId);
    gpLog_Flush();
}
#endif //GP_LOCAL_LOG

void BleConfig_SetEventMask(gpHci_EventMask_t* pDst, gpHci_EventMask_t* pSrc)
{
   MEMCPY(pDst, pSrc, BLE_EVENT_MASK_SIZE);
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleConfig_Init(void)
{
    // Populate the default features this device supports
    BleConfig_InitLocalSupportedFeaturesMask();

}

void gpBleConfig_Reset(Bool firstReset)
{

#ifdef GP_LOCAL_LOG
    if(firstReset)
    {
        BleConfig_DumpConfig();
    }
#endif //GP_LOCAL_LOG
    Ble_ConfigContext.isRandomAddressSet=false;


    // Initialize event masks to their defaults
    Ble_ConfigContext.cbEventMask = GPBLECONFIG_EVENT_MASK_DEFAULT;
    Ble_ConfigContext.cbEventMaskPage2 = GPBLECONFIG_EVENT_MASK_PAGE2_DEFAULT;
    Ble_ConfigContext.leEventMask = GPBLECONFIG_EVENT_MASK_LE_DEFAULT;
    Ble_ConfigContext.cbVsdEventMask = GPBLECONFIG_VSD_EVENT_MASK_DEFAULT;



}

void gpBleConfig_GetEventMask(gpHci_EventMask_t* pMask, gpHci_EventMaskType_t type)
{
    switch(type)
    {
        case gpHci_CB:
        {
            MEMCPY(pMask, &Ble_ConfigContext.cbEventMask, BLE_EVENT_MASK_SIZE);
            break;
        }
        case gpHci_CBPage2:
        {
            MEMCPY(pMask, &Ble_ConfigContext.cbEventMaskPage2, BLE_EVENT_MASK_SIZE);
            break;
        }
        case gpHci_LEMeta:
        {
            MEMCPY(pMask, &Ble_ConfigContext.leEventMask, BLE_EVENT_MASK_SIZE);
            break;
        }
        default:
        {
            GP_ASSERT_DEV_INT(false);
            break;
        }
    }
}


Bool BleConfig_GetRandomAddress(BtDeviceAddress_t* pAddress)
{
    if (!Ble_ConfigContext.isRandomAddressSet)
    {
        return false;
    }

    MEMCPY(pAddress->addr, &Ble_ConfigContext.randomAddress, sizeof(BtDeviceAddress_t));
    return true;
}

gpHci_Result_t BleConfig_SetRandomAddressChecker(void)
{

#ifdef GP_DIVERSITY_BLE_ADVERTISER
    // Only disallowed when legacy advertising is started with a legacy command
    // (ext adv sets have a dedicated LE Set Advertising Set Random Address command)
    if(gpBleAdvertiser_IsEnabled())
    {
        GP_LOG_PRINTF("Set random address not allowed during adv",0);
        return gpHci_ResultCommandDisallowed;
    }
#endif // GP_DIVERSITY_BLE_ADVERTISER

#ifdef GP_DIVERSITY_BLE_SCANNER
    // check if legacy- or ext- scanning is enabled
    if(gpBleScanner_IsEnabled())
    {
        GP_LOG_PRINTF("Set random address not allowed during scan",0);
        return gpHci_ResultCommandDisallowed;
    }
#endif // GP_DIVERSITY_BLE_SCANNER

#ifdef GP_DIVERSITY_BLE_INITIATOR
    // check if legacy- or ext- initiator-scanning is enabled
    if(gpBleInitiator_IsEnabled())
    {
        GP_LOG_PRINTF("Set random address not allowed during init",0);
        return gpHci_ResultCommandDisallowed;
    }
#endif // GP_DIVERSITY_BLE_INITIATOR

    return gpHci_ResultSuccess;
}

#if defined(GP_DIVERSITY_BLE_MASTER) 
gpHci_Result_t Ble_SetHostChannelClassificationChecker(gpHci_ChannelMap_t* pHostChannelMap)
{
    UInt8 nrOfUsedChannels = 0;


    // check that at least 2 channels are in use: BT v4.2 Vol6 Part B $4.5.8.1
    nrOfUsedChannels = Ble_CountChannels(pHostChannelMap);

    if(nrOfUsedChannels < 2)
    {
        GP_LOG_PRINTF("less than 2 channels", 0);
        return gpHci_ResultInvalidHCICommandParameters;
    }

    return gpHci_ResultSuccess;
}
#endif /* GP_DIVERSITY_BLE_MASTER or GP_DIVERSITY_EXTENDED_ADVERTISING */


/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

void gpBleConfig_RegisterHciResetCallback(gpBle_HciResetCallback_t cb)
{
    MEMCPY(&Ble_ConfigContext.hciResetCb, &cb, sizeof(cb));
}

gpHci_Result_t gpBleConfig_GetOwnAddress(BtDeviceAddress_t* pAddress, gpHci_OwnAddressType_t ownAddressType)
{
    if(!BLE_OWN_ADDRESS_TYPE_VALID(ownAddressType))
    {
        return gpHci_ResultInvalidHCICommandParameters;
    }

    if(ownAddressType == gpHci_OwnAddressType_PublicDevice || ownAddressType == gpHci_OwnAddressType_RPAPublicIfUnavailable)
    {
        gpHal_BleGetDeviceAddress(pAddress);

        return gpHci_ResultSuccess;
    }
    else if(ownAddressType == gpHci_OwnAddressType_RandomDevice || ownAddressType == gpHci_OwnAddressType_RPARandomIfUnavailable)
    {
        // Use the Random address allocated by the host
        if (BleConfig_GetRandomAddress(pAddress))
        {
            return gpHci_ResultSuccess;
        }
        else
        {
            return gpHci_ResultInvalidHCICommandParameters;
        }
    }
    else
    {
        // Cannot happen, checked before
        GP_ASSERT_DEV_INT(false);
        return gpHci_ResultInvalidHCICommandParameters;
    }
}

Bool gpBleConfig_HasRandomAddress(void)
{
    return Ble_ConfigContext.isRandomAddressSet;
}

void gpBleConfig_GetLocalSupportedFeatures(UInt64* pFeatures)
{
    GP_ASSERT_DEV_INT(pFeatures != NULL);

    MEMCPY(pFeatures, &Ble_LocalSupportedFeatures, sizeof(Ble_LocalSupportedFeatures));
}

Bool gpBleConfig_IsEventMasked(gpHci_EventCode_t event)
{
    if( event == gpHci_EventCode_NumberOfCompletedPackets   ||
        event == gpHci_EventCode_CommandComplete            ||
        event == gpHci_EventCode_CommandStatus              ||
        event == gpHci_EventCode_HardwareError              ||
        event == gpHci_EventCode_VsdSinkRxIndication        ||
        // There is no mask bit for VSD debug events, so we always need to forward them,and then we check them in gpBleConfig_IsLeMetaVSDEventMasked
        event == gpHci_EventCode_VsdMeta
      )
    {
        // Some special events should never be masked
        return true;
    }

    if(event < GPBLECONFIG_NR_OF_EVENTS_IN_MASK)
    {
        // Event bit is located in normal event mask (page 1)
        return BIT_TST64(Ble_ConfigContext.cbEventMask, (event - 1));
    }
    else if(event - GPBLECONFIG_NR_OF_EVENTS_IN_MASK < GPBLECONFIG_NR_OF_EVENTS_IN_MASK)
    {
        // Event bit is located in second page of event masks
        UInt8 bitNumber = (event - GPBLECONFIG_NR_OF_EVENTS_IN_MASK);

        return BIT_TST64(Ble_ConfigContext.cbEventMaskPage2, bitNumber);
    }
    else
    {
        // Invalid event number
        return false;
    }
}

Bool gpBleConfig_IsLeMetaEventMasked(gpHci_LEMetaSubEventCode_t event)
{
    return BIT_TST(Ble_ConfigContext.leEventMask, (event - 1));
}

Bool gpBleConfig_IsLeMetaVSDEventMasked(gpHci_VsdSubEventCode_t event)
{
    UInt16    mask = BM(event-1);

    if((Ble_ConfigContext.cbVsdEventMask & mask) != 0)
    {
        return true;
    }

    return false;
}

gpHci_Result_t gpBle_SetLeMetaVSDEvent(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    result = gpHci_ResultSuccess;

    Ble_ConfigContext.cbVsdEventMask |= BM(pParams->SetLeMetaVSDEvent.eventCode-1);

    return result;

}

gpHci_Result_t gpBle_ResetLeMetaVSDEvent(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    result = gpHci_ResultSuccess;

    Ble_ConfigContext.cbVsdEventMask &= ~(BM(pParams->ResetLeMetaVSDEvent.eventCode-1));
    return result;

}





/*****************************************************************************
 *                    Public Service Function Definitions
 *****************************************************************************/

gpHci_Result_t gpBle_Reset(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    // Reset order: HCI - gpHal - Link layer

    if(Ble_ConfigContext.hciResetCb != NULL)
    {
        Ble_ConfigContext.hciResetCb();
    }


    gpBleComps_ResetLinkLayer();

    return gpHci_ResultSuccess;
}


gpHci_Result_t gpBle_ReadLocalVersionInformation(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpBle_Configuration_t localConfig;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    gpBleConfig_GetConfig(&localConfig);

    pEventBuf->payload.commandCompleteParams.returnParams.leReadLocalVersion.hciVersion = localConfig.hciVersion;
    pEventBuf->payload.commandCompleteParams.returnParams.leReadLocalVersion.hciRevision = localConfig.hciRevision;
    pEventBuf->payload.commandCompleteParams.returnParams.leReadLocalVersion.lmppalVersion = localConfig.lmppalVersion;
    pEventBuf->payload.commandCompleteParams.returnParams.leReadLocalVersion.manufacturerName = localConfig.companyId;
    pEventBuf->payload.commandCompleteParams.returnParams.leReadLocalVersion.lmppalSubversion = localConfig.lmppalSubversion;

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_ReadLocalSupportedCommands(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    MEMSET(&pEventBuf->payload.commandCompleteParams.returnParams.supportedCommands, 0 , GP_HCI_FEATURES_COMMANDS_SIZE);
    MEMCPY_P(&pEventBuf->payload.commandCompleteParams.returnParams.supportedCommands, BleConfig_SupportedCommands, BLECONFIG_SUPPORTED_COMMANDS_LENGTH);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_ReadLocalSupportedFeatures(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    MEMSET(&pEventBuf->payload.commandCompleteParams.returnParams.supportedFeatures.supportedFeatures[0], 0 , GP_HCI_FEATURES_LIST_SIZE);

    pEventBuf->payload.commandCompleteParams.returnParams.supportedFeatures.supportedFeatures[4] = 0x60; // bit37 bit38 set : BR/EDR not supported | LE supported
    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_ReadBufferSize(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    // This is just an example of how to reject a "not supported" HCI command
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    return gpHci_ResultUnknownHCICommand;
}

gpHci_Result_t gpBle_ReadBdAddr(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    return gpBleConfig_GetOwnAddress(&pEventBuf->payload.commandCompleteParams.returnParams.bdAddress, gpHci_OwnAddressType_PublicDevice);
}

gpHci_Result_t gpBle_LeSetRandomAddress(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    result = BleConfig_SetRandomAddressChecker();

    if(result == gpHci_ResultSuccess)
    {
        MEMCPY(&Ble_ConfigContext.randomAddress, &pParams->LeSetRandomAddress.address, sizeof(BtDeviceAddress_t));
        Ble_ConfigContext.isRandomAddressSet = true;
    }

    return result;
}


gpHci_Result_t gpBle_VsdWriteDeviceAddress(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    gpHal_BleSetDeviceAddress(&pParams->VsdWriteDeviceAddress.address);

    return gpHci_ResultSuccess;
}

// gpBle_(Le)ReadLocalSupportedStates is now implemented in gpBle_LLCP.c

gpHci_Result_t gpBle_LeReadSupportedStates(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    // We support all state combinations (incl low duty cycle directed adv) mentioned in the spec Vol 2 Part E $7.8.27
    // So we want to return the following state-map : (lsb to msb) FF FF FF FF FF 03 00 00
    MEMSET(&pEventBuf->payload.commandCompleteParams.returnParams.supportedFeatures.supportedFeatures[0], 0xFF , 5);
    pEventBuf->payload.commandCompleteParams.returnParams.supportedFeatures.supportedFeatures[5] = 0x03;
    MEMSET(&pEventBuf->payload.commandCompleteParams.returnParams.supportedFeatures.supportedFeatures[6], 0 , 2);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_LeReadLocalSupportedFeatures(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    MEMCPY(pEventBuf->payload.commandCompleteParams.returnParams.supportedFeatures.supportedFeatures, (UInt8*)&Ble_LocalSupportedFeatures, GP_HCI_FEATURE_SET_SIZE);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_SetEventMask(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    BleConfig_SetEventMask(&Ble_ConfigContext.cbEventMask, &pParams->SetEventMask.eventMask);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_SetEventMaskPage2(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    BleConfig_SetEventMask(&Ble_ConfigContext.cbEventMaskPage2, &pParams->SetEventMaskPage2.eventMask);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_LeSetEventMask(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    BleConfig_SetEventMask(&Ble_ConfigContext.leEventMask, &pParams->LeSetEventMask.eventMask);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_UnknownOpCode(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    return gpHci_ResultUnknownHCICommand;
}

#ifdef GP_DIVERSITY_DEVELOPMENT
gpHci_Result_t gpBle_VsdGetBuildP4Changelist(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    pEventBuf->payload.commandCompleteParams.returnParams.vsdGetBuildP4Changelist.changelistNumber = (UInt32)GP_CHANGELIST;
    return gpHci_ResultSuccess;
}
#endif // GP_DIVERSITY_DEVELOPMENT

gpHci_Result_t gpBle_LeReadRfPathCompensation(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    Int8 TxPathCompensation, RxPathCompensation;

    gpHal_BleGetRfPathCompensation(&TxPathCompensation, &RxPathCompensation);
    pEventBuf->payload.commandCompleteParams.returnParams.LeReadRfPathCompensation.RF_Tx_Path_Compensation_Value = BLECONFIG_RFPATHCOMP_DB_TO_UNITS(TxPathCompensation);
    pEventBuf->payload.commandCompleteParams.returnParams.LeReadRfPathCompensation.RF_Rx_Path_Compensation_Value = BLECONFIG_RFPATHCOMP_DB_TO_UNITS(RxPathCompensation);

    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_LeWriteRfPathCompensation(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    if ( BLE_RANGE_CHECK(pParams->LeWriteRfPathCompensation.RF_Tx_Path_Compensation_Value, BLECONFIG_RFPATHCOMP_MIN_UNITS, BLECONFIG_RFPATHCOMP_MAX_UNITS ) &&
         BLE_RANGE_CHECK(pParams->LeWriteRfPathCompensation.RF_Rx_Path_Compensation_Value, BLECONFIG_RFPATHCOMP_MIN_UNITS, BLECONFIG_RFPATHCOMP_MAX_UNITS ) )
    {
        gpHal_BleSetRfPathCompensation(BLECONFIG_RFPATHCOMP_UNITS_TO_DB(pParams->LeWriteRfPathCompensation.RF_Tx_Path_Compensation_Value),
                                       BLECONFIG_RFPATHCOMP_UNITS_TO_DB(pParams->LeWriteRfPathCompensation.RF_Rx_Path_Compensation_Value));
        return gpHci_ResultSuccess;
    }
    return gpHci_ResultInvalidHCICommandParameters;
}

gpHci_Result_t gpBle_LeReadTransmitPower(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    result = gpHci_ResultSuccess;

    gpHal_BleGetMinMaxPowerLevels(&pEventBuf->payload.commandCompleteParams.returnParams.LeReadTransmitPower.Min_Tx_Power,
                                  &pEventBuf->payload.commandCompleteParams.returnParams.LeReadTransmitPower.Max_Tx_Power);


    return result;
}

#if defined(GP_DIVERSITY_BLE_MASTER) 
gpHci_Result_t gpBle_LeSetHostChannelClassification(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    gpHci_Result_t result;

    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    // 3 MSBs are reserved (cfr Vol 2 Part E $ 7.8.19) - must be ignored on reception
    pParams->LeSetHostChannelClassification.channels.channels[BLE_CHANNEL_MAP_SIZE-1] &= GP_BLE_CHANNEL_MAP_MSB_MASK;
    result = Ble_SetHostChannelClassificationChecker(&pParams->LeSetHostChannelClassification.channels);

    if(result == gpHci_ResultSuccess)
    {
#ifdef GP_DIVERSITY_BLE_MASTER
        if (gpBle_IsMasterChannelMapUpdateInProgress())
        {
            return gpHci_ResultMemoryCapacityExceeded;
        }
#endif /* GP_DIVERSITY_BLE_MASTER */



#ifdef GP_DIVERSITY_BLE_MASTER
        result = gpBle_SetNewMasterChannelMap(&pParams->LeSetHostChannelClassification.channels);
#endif /* GP_DIVERSITY_BLE_MASTER */

    }

    return result;
}
#endif // GP_DIVERSITY_BLE_MASTER or GP_DIVERSITY_EXTENDED_ADVERTISING

#ifdef GP_DIVERSITY_DEVELOPMENT
gpHci_Result_t gpBle_VsdGetRtMgrVersion(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);
    pEventBuf->payload.commandCompleteParams.returnParams.RtMgrVersion.version = gpHal_BleGetRtBleMgrVersion();
    return gpHci_ResultSuccess;
}

gpHci_Result_t gpBle_SetVsdOverruleLocalSupportedFeaturesHelper(gpHci_CommandParameters_t* pParams, gpBle_EventBuffer_t* pEventBuf)
{
    BLE_SET_RESPONSE_EVENT_COMMAND_COMPLETE(pEventBuf->eventCode);

    MEMCPY(&Ble_LocalSupportedFeatures, pParams->SetVsdTestParams.value, pParams->SetVsdTestParams.length);

    return gpHci_ResultSuccess;
}
#endif /* GP_DIVERSITY_DEVELOPMENT */
