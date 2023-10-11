/*
 * Copyright (c) 2020, Qorvo Inc
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
 */

/** @file "qorvo_config.h"
 *
 */

#ifndef _QORVO_CONFIG_H_
#define _QORVO_CONFIG_H_

/*
 * Version info
 */

#define GP_CHANGELIST                                                                   0
#define GP_VERSIONINFO_APP                                                              ble_peripheral_qpg6105_release
#define GP_VERSIONINFO_BASE_COMPS                                                       0,0,0,0
#define GP_VERSIONINFO_BLE_COMPS                                                        0,0,0,0
#define GP_VERSIONINFO_DATE                                                             2023-10-11
#define GP_VERSIONINFO_GLOBAL_VERSION                                                   1,0,0,0
#define GP_VERSIONINFO_HOST                                                             UNKNOWN
#define GP_VERSIONINFO_PROJECT                                                          P345_Matter_DK_Endnodes
#define GP_VERSIONINFO_USER                                                             UNKNOWN@UNKNOWN


/*
 * Component: ble_peripheral
 */

/* Add BLE Peripheral functionality to application */
#define GP_APP_DIVERSITY_BLE_PERIPHERAL


/*
 * Component: BleModule
 */

/* BLE peripheral role */
#define GP_BLEMODULE_PERIPHERAL


/*
 * Component: cordioAppFramework
 */

/* Use Attribute Server functionality */
#define CORDIO_APPFRAMEWORK_DIVERSITY_ATT_SERVER


/*
 * Component: gpAssert
 */

/* Choose reset as default assert action */
#define GP_DIVERSITY_ASSERT_ACTION_RESET


/*
 * Component: gpBle
 */

/* Buffer size for HCI RX packets */
#define GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_RX                                             251

/* Buffer size for HCI TX packets */
#define GP_BLE_DIVERSITY_HCI_BUFFER_SIZE_TX                                             251

/* WcBleHost will be calling gpBle_ExecuteCommand */
#define GP_DIVERSITY_BLE_EXECUTE_CMD_WCBLEHOST


/*
 * Component: gpBleComps
 */

/* The amount of dedicated connection complete buffers */
#define GP_BLE_NR_OF_CONNECTION_COMPLETE_EVENT_BUFFERS                                  1


/*
 * Component: gpBleConfig
 */

/* The amount of LLCP procedures that are supported */
#define GP_BLE_NR_OF_SUPPORTED_PROCEDURES                                               11

/* Bluetooth spec version (LL and HCI) */
#define GP_DIVERSITY_BLECONFIG_VERSION_ID                                               gpBleConfig_BleVersionId_5_3


/*
 * Component: gpBleConnectionQueue
 */

/* Advertising Queue Id */
#define ADVQUEUE_ID                                                                     0


/*
 * Component: gpBleLlcpFramework
 */

/* The amount of LLCP procedures callbacks that are supported */
#define GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS                                      0


/*
 * Component: gpBsp
 */

/* Contains filename of BSP header file to include */
#define GP_BSP_FILENAME                                                                 "gpBsp_QPG6105DK_B01.h"

/* UART baudrate */
#define GP_BSP_UART_COM_BAUDRATE                                                        115200


/*
 * Component: gpCom
 */

/* Multiple gpComs were specified - defined in code */
#define GP_COM_DIVERSITY_MULTIPLE_COM

/* Enable SYN datastream encapsulation */
#define GP_COM_DIVERSITY_SERIAL

/* Maximum amount of modules supported for Rx handling by gpCom. Environment already calculates minimal required module ID's */
#define GP_COM_MAX_NUMBER_OF_MODULE_IDS                                                 2

#define GP_COM_MAX_PACKET_PAYLOAD_SIZE                                                  250

/* Use UART for COM - defined as default in code */
#define GP_DIVERSITY_COM_UART


/*
 * Component: gphal
 */

/* Max BLE connections supported */
#define GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS                                3

/* Max BLE slave connections supported */
#define GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS                          3

/* Enables a callback to be called after 32kHz calibration. */
#define GP_DIVERSITY_GPHAL_32KHZ_CALIBRATION_DONE_CB

/* Call update callback anytime the index is modified */
#define GP_DIVERSITY_GPHAL_WHITELIST_UPDATE_CALLBACK

/* Number of PBMS of first supported size */
#define GP_HAL_PBM_TYPE1_AMOUNT                                                         8

/* Number of PBMS of second supported size */
#define GP_HAL_PBM_TYPE2_AMOUNT                                                         10


/*
 * Component: gpJumpTables_k8e
 */

/* gp scheduler ROM ver 2.0 */
#define GP_DIVERSITY_ROM_GPSCHED_V2


/*
 * Component: gpLog
 */

/* overrule log string length from application code */
#define GP_LOG_MAX_LEN                                                                  256


/*
 * Component: gpNvm
 */

/* Size of reserved section for NVM */
#define GP_DATA_SECTION_SIZE_NVM                                                        0x4000

/* Maximum number of unique tags in each pool. Used for memory allocation at Tag level API */
#define GP_NVM_NBR_OF_UNIQUE_TAGS                                                       32

#define GP_NVM_TYPE                                                                     6


/*
 * Component: gpSched
 */

#define GP_SCHED_EVENT_LIST_SIZE                                                        20

/* Callback after every main loop iteration. */
#define GP_SCHED_NR_OF_IDLE_CALLBACKS                                                   1


/*
 * Component: gpUpgrade
 */

/* Don't use gpExtStorage, even if available */
#define GP_UPGRADE_DIVERSITY_USE_INTSTORAGE


/*
 * Component: halCortexM4
 */

/* Use extended loaded user license */
#define GP_DIVERSITY_EXTENDED_USER_LICENSE

/* Use loaded user license */
#define GP_DIVERSITY_LOADED_USER_LICENSE

/* Set if hal has real mutex capability. Used to skip even disabling/enabling global interrupts. */
#define HAL_MUTEX_SUPPORTED


/*
 * Component: qorvoBleHost
 */

/* The MTU size */
#define CORDIO_BLE_HOST_ATT_MAX_MTU                                                     517

/* Number of chuncks WSF Poolmem Chunk 5 */
#define CORDIO_BLE_HOST_BUFPOOLS_5_AMOUNT                                               5

/* Size of WSF Poolmem Chunk 5, must be larger as Chunk 4 */
#define CORDIO_BLE_HOST_BUFPOOLS_5_CHUNK                                                534

/* Cordio define - checked here */
#define DM_CONN_MAX                                                                     3


/*
 * Component: silexCryptoSoc
 */

#define AES_GCM_EMABLED                                                                 0

#define AES_HW_KEYS_ENABLED                                                             0

#define AES_MASK_ENABLED                                                                0

/* Include curve, optimization diversity, to exclude other curves from taking up flash space */
#define GP_SILEXCRYPTOSOC_DIVERSITY_INCLUDE_SX_ECP_DP_SECP256R1


#include "qorvo_internals.h"

#endif //_QORVO_CONFIG_H_
