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
#define GP_VERSIONINFO_APP                                                              PTC_QPG6105_10DBM_UART8to9
#define GP_VERSIONINFO_BASE_COMPS                                                       0,0,0,0
#define GP_VERSIONINFO_BLE_COMPS                                                        0,0,0,0
#define GP_VERSIONINFO_DATE                                                             2023-05-16
#define GP_VERSIONINFO_GLOBAL_VERSION                                                   1,0,0,0
#define GP_VERSIONINFO_HOST                                                             UNKNOWN
#define GP_VERSIONINFO_PROJECT                                                          P345_Matter_DK_Endnodes
#define GP_VERSIONINFO_USER                                                             UNKNOWN@UNKNOWN


/*
 * Component: gpBle
 */

/* gpHci Wrapper will be calling gpBle_ExecuteCommand */
#define GP_DIVERSITY_BLE_EXECUTE_CMD_HCIWRAPPER


/*
 * Component: gpBleComps
 */

/* The amount of dedicated connection complete buffers */
#define GP_BLE_NR_OF_CONNECTION_COMPLETE_EVENT_BUFFERS                                  1


/*
 * Component: gpBleConfig
 */

/* The amount of LLCP procedures that are supported */
#define GP_BLE_NR_OF_SUPPORTED_PROCEDURES                                               6

/* Bluetooth spec version (LL and HCI) */
#define GP_DIVERSITY_BLECONFIG_VERSION_ID                                               gpBleConfig_BleVersionId_5_3


/*
 * Component: gpBleLlcpFramework
 */

/* The amount of LLCP procedures callbacks that are supported */
#define GP_BLE_NR_OF_SUPPORTED_PROCEDURE_CALLBACKS                                      0


/*
 * Component: gpBleTest
 */

/* Marshalling used on tbc */
#define GP_BLETEST_DIVERSITY_MARSHAL


/*
 * Component: gpBsp
 */

/* Contains filename of BSP header file to include */
#define GP_BSP_FILENAME                                                                 "gpBsp_QPG6105DK_B01.h"


/*
 * Component: gpCom
 */

/* To be specified when using multiple comms */
#define GP_COM_DEFAULT_COMMUNICATION_ID                                                 GP_COM_COMM_ID_UART1

/* Multiple gpComs were specified - defined in code */
#define GP_COM_DIVERSITY_MULTIPLE_COM

/* Enable SYN datastream encapsulation */
#define GP_COM_DIVERSITY_SERIAL

/* Maximum amount of modules supported for Rx handling by gpCom. Environment already calculates minimal required module ID's */
#define GP_COM_MAX_NUMBER_OF_MODULE_IDS                                                 2

#define GP_COM_MAX_PACKET_PAYLOAD_SIZE                                                  450

#define GP_COM_MAX_TX_BUFFER_SIZE                                                       1024

#define GP_COM_RX_PACKET_BUFFERS                                                        5

/* Use UART for COM - defined as default in code */
#define GP_DIVERSITY_COM_UART


/*
 * Component: gphal
 */

/* Max BLE connections supported */
#define GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS                                1

/* Max BLE slave connections supported */
#define GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_SLAVE_CONNECTIONS                          1

/* Enables a callback to be called after 32kHz calibration. */
#define GP_DIVERSITY_GPHAL_32KHZ_CALIBRATION_DONE_CB

/* Activate XTAL32M Capacitor trimming and AGC level tuning */
#define GP_DIVERSITY_GPHAL_TRIM_XTAL_32M

/* Do CSMA-CA in software */
#define GP_HAL_MAC_SW_CSMA_CA

/* Number of PBMS of first supported size */
#define GP_HAL_PBM_TYPE1_AMOUNT                                                         3

/* Number of PBMS of second supported size */
#define GP_HAL_PBM_TYPE2_AMOUNT                                                         3


/*
 * Component: gpHci
 */

#define GP_HCI_COMM_ID                                                                  GP_COM_COMM_ID_EVENT_UART1


/*
 * Component: gpLog
 */

/* CommId to send out logging to */
#define GP_LOG_COMMUNICATION_ID                                                         GP_COM_COMM_ID_UART1


/*
 * Component: gpPoolMem
 */

/* Amount of Chunks 1 */
#define GP_POOLMEM_CHUNK_AMOUNT1                                                        10

/* Amount of Chunks 3 */
#define GP_POOLMEM_CHUNK_AMOUNT3                                                        3

/* Size of Chunks 3 */
#define GP_POOLMEM_CHUNK_SIZE3                                                          260


/*
 * Component: gpPTC
 */

#define GP_PTC_DIVERSITY_GPCOM_SERVER

/* generated wrappers */
#define GP_PTC_DIVERSITY_MARSHAL

/* Productname used for PTC identicifation */
#define GP_PTC_PRODUCTNAME                                                              PTC_QPG6105_10DBM_UART8to9

#define GP_PTC_VERSION                                                                  2,0,0,0


/*
 * Component: gpRadio
 */

/* marshalling flag */
#define GP_RADIO_DIVERSITY_MARSHAL


/*
 * Component: gpSched
 */

#define GP_SCHED_EVENT_LIST_SIZE                                                        16

/* Callback after every main loop iteration. */
#define GP_SCHED_NR_OF_IDLE_CALLBACKS                                                   0


/*
 * Component: gpTest
 */

/* Marshalling used on tbc */
#define GP_TEST_DIVERSITY_MARSHAL


#include "qorvo_internals.h"

#endif //_QORVO_CONFIG_H_
