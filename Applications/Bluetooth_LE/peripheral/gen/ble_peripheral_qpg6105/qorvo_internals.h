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

/** @file "qorvo_internals.h"
 *
 */

#ifndef _QORVO_INTERNALS_H_
#define _QORVO_INTERNALS_H_

/*
 * Enabled components
 */

#define GP_COMP_ASSERT
#define GP_COMP_BASECOMPS
#define GP_COMP_BLE
#define GP_COMP_BLEACTIVITYMANAGER
#define GP_COMP_BLEADDRESSRESOLVER
#define GP_COMP_BLEADVERTISER
#define GP_COMP_BLECOMPS
#define GP_COMP_BLECONFIG
#define GP_COMP_BLECONNECTIONMANAGER
#define GP_COMP_BLECONNECTIONQUEUE
#define GP_COMP_BLEDATACHANNELRXQUEUE
#define GP_COMP_BLEDATACHANNELTXQUEUE
#define GP_COMP_BLEDATACOMMON
#define GP_COMP_BLEDATARX
#define GP_COMP_BLEDATATX
#define GP_COMP_BLELLCP
#define GP_COMP_BLELLCPFRAMEWORK
#define GP_COMP_BLELLCPPROCEDURES
#define GP_COMP_BLEMODULE
#define GP_COMP_BLEPERIPHERALCONNECTIONSTM
#define GP_COMP_BLERESPRADDR
#define GP_COMP_BLESECURITYCOPROCESSOR
#define GP_COMP_BLETESTMODE
#define GP_COMP_BSP
#define GP_COMP_COM
#define GP_COMP_CORDIOAPPFRAMEWORK
#define GP_COMP_CORDIOSERVICESPROFILES
#define GP_COMP_ECC
#define GP_COMP_ENCRYPTION
#define GP_COMP_FREERTOS
#define GP_COMP_GPHAL
#define GP_COMP_GPHAL_BLE
#define GP_COMP_GPHAL_PBM
#define GP_COMP_GPHAL_RADIO
#define GP_COMP_GPHAL_SEC
#define GP_COMP_HALCORTEXM4
#define GP_COMP_HCI
#define GP_COMP_JUMPTABLES_K8E
#define GP_COMP_LOG
#define GP_COMP_NVM
#define GP_COMP_OTA
#define GP_COMP_PD
#define GP_COMP_POOLMEM
#define GP_COMP_QORVOBLEHOST
#define GP_COMP_RANDOM
#define GP_COMP_RESET
#define GP_COMP_RT_NRT_COMMON
#define GP_COMP_SCHED
#define GP_COMP_SILEXCRYPTOSOC
#define GP_COMP_TLS
#define GP_COMP_UPGRADE
#define GP_COMP_UTILS
#define GP_COMP_VERSION
#define GP_COMP_WMRK

/*
 * Components numeric ids
 */

#define GP_COMPONENT_ID_APP                                                             1
#define GP_COMPONENT_ID_ASSERT                                                          29
#define GP_COMPONENT_ID_BASECOMPS                                                       35
#define GP_COMPONENT_ID_BLE                                                             154
#define GP_COMPONENT_ID_BLEACTIVITYMANAGER                                              228
#define GP_COMPONENT_ID_BLEADDRESSRESOLVER                                              214
#define GP_COMPONENT_ID_BLEADVERTISER                                                   215
#define GP_COMPONENT_ID_BLECOMPS                                                        216
#define GP_COMPONENT_ID_BLECONFIG                                                       217
#define GP_COMPONENT_ID_BLECONNECTIONMANAGER                                            75
#define GP_COMPONENT_ID_BLECONNECTIONQUEUE                                              19
#define GP_COMPONENT_ID_BLEDATACHANNELRXQUEUE                                           218
#define GP_COMPONENT_ID_BLEDATACHANNELTXQUEUE                                           219
#define GP_COMPONENT_ID_BLEDATACOMMON                                                   220
#define GP_COMPONENT_ID_BLEDATARX                                                       221
#define GP_COMPONENT_ID_BLEDATATX                                                       222
#define GP_COMPONENT_ID_BLEINITIATOR                                                    223
#define GP_COMPONENT_ID_BLELLCP                                                         224
#define GP_COMPONENT_ID_BLELLCPFRAMEWORK                                                225
#define GP_COMPONENT_ID_BLELLCPPROCEDURES                                               226
#define GP_COMPONENT_ID_BLEMODULE                                                       241
#define GP_COMPONENT_ID_BLEPERIPHERALCONNECTIONSTM                                      238
#define GP_COMPONENT_ID_BLEPRESCHED                                                     234
#define GP_COMPONENT_ID_BLERESPRADDR                                                    233
#define GP_COMPONENT_ID_BLESECURITYCOPROCESSOR                                          229
#define GP_COMPONENT_ID_BLETESTMODE                                                     230
#define GP_COMPONENT_ID_BSP                                                             8
#define GP_COMPONENT_ID_COM                                                             10
#define GP_COMPONENT_ID_CORDIOAPPFRAMEWORK                                              236
#define GP_COMPONENT_ID_CORDIOSERVICESPROFILES                                          237
#define GP_COMPONENT_ID_ECC                                                             192
#define GP_COMPONENT_ID_ENCRYPTION                                                      124
#define GP_COMPONENT_ID_FREERTOS                                                        24
#define GP_COMPONENT_ID_GPHAL                                                           7
#define GP_COMPONENT_ID_HALCORTEXM4                                                     6
#define GP_COMPONENT_ID_HCI                                                             156
#define GP_COMPONENT_ID_JUMPTABLES_K8E                                                  60
#define GP_COMPONENT_ID_LOG                                                             11
#define GP_COMPONENT_ID_NVM                                                             32
#define GP_COMPONENT_ID_OTA                                                             39
#define GP_COMPONENT_ID_PAD                                                             126
#define GP_COMPONENT_ID_PD                                                              104
#define GP_COMPONENT_ID_POOLMEM                                                         106
#define GP_COMPONENT_ID_QORVOBLEHOST                                                    185
#define GP_COMPONENT_ID_RANDOM                                                          108
#define GP_COMPONENT_ID_RESET                                                           33
#define GP_COMPONENT_ID_RT_NRT_COMMON                                                   -1
#define GP_COMPONENT_ID_RXARBITER                                                       2
#define GP_COMPONENT_ID_SCHED                                                           9
#define GP_COMPONENT_ID_SILEXCRYPTOSOC                                                  54
#define GP_COMPONENT_ID_STAT                                                            22
#define GP_COMPONENT_ID_TLS                                                             14
#define GP_COMPONENT_ID_UPGRADE                                                         115
#define GP_COMPONENT_ID_UTILS                                                           4
#define GP_COMPONENT_ID_VERSION                                                         129
#define GP_COMPONENT_ID_WMRK                                                            51

/*
 * Component: ble_peripheral
 */

#define GP_DATA_SECTION_NAME_JTOTA                                                      JTOTA
#define GP_DATA_SECTION_SIZE_JTOTA                                                      0x1000

/*
 * Component: BleModule
 */

#define GP_BLEPERIPHERAL_DIVERSITY_WDXS
#define GP_BLE_ATT_SERVER_DAT

/*
 * Component: cordioAppFramework
 */

#define GP_DIVERSITY_CORDIO_QORVO_APP

/*
 * Component: cordioServicesProfiles
 */

#define CORDIO_SERVICE_WP

/*
 * Component: gpBle
 */

#define GP_BLE_DIVERSITY_ENHANCED_CONNECTION_COMPLETE

/*
 * Component: gpBleAddressResolver
 */

#define GP_ROM_PATCHED_Ble_ClearFilterAcceptList
#define GP_ROM_PATCHED_Ble_ManipulateFilterAcceptListAllowedChecker
#define GP_ROM_PATCHED_Ble_ManipulateFilterAcceptListChecker
#define GP_ROM_PATCHED_gpBleAddressResolver_EnableConnectedDevicesInFilterAcceptList
#define GP_ROM_PATCHED_gpBleAddressResolver_UpdateFilterAcceptListEntryState
#define GP_ROM_PATCHED_gpBleAddressResolver_UpdateFilterAcceptListEntryStateBulk
#define GP_ROM_PATCHED_gpBle_LeAddDeviceToFilterAcceptList
#define GP_ROM_PATCHED_gpBle_LeRemoveDeviceFromFilterAcceptList

/*
 * Component: gpBleComps
 */

#define GP_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
#define GP_DIVERSITY_BLE_ACL_CONNECTIONS_SUPPORTED
#define GP_DIVERSITY_BLE_AUTHENTICATED_PAYLOAD_TO_SUPPORTED
#define GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED
#define GP_DIVERSITY_BLE_CONN_PARAM_REQUEST_SUPPORTED
#define GP_DIVERSITY_BLE_DATA_LENGTH_UPDATE_SUPPORTED
#define GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
#define GP_DIVERSITY_BLE_ENCRYPTION_SUPPORTED
#define GP_DIVERSITY_BLE_EXTENDED_REJECT_SUPPORTED
#define GP_DIVERSITY_BLE_EXT_SCAN_FILTER_POLICIES_SUPPORTED
#define GP_DIVERSITY_BLE_LEGACY_ADVERTISING
#define GP_DIVERSITY_BLE_LINK_LAYER_PRIVACY_SUPPORTED
#define GP_DIVERSITY_BLE_PERIPHERAL
#define GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED
#define GP_DIVERSITY_BLE_PING_SUPPORTED
#define GP_DIVERSITY_BLE_SLAVE_FEAT_EXCHANGE_SUPPORTED

/*
 * Component: gpBsp
 */

#define GP_DIVERSITY_QPG6105DK_B01

/*
 * Component: gpCom
 */

#define GP_COM_DIVERSITY_ACTIVATE_TX_CALLBACK
#define GP_COM_DIVERSITY_SERIAL_NO_SYN_NO_CRC
#define GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID                                        18
#define GP_COM_DIVERSITY_SERIAL_SYN_DISABLED

/*
 * Component: gpECC
 */

#define GP_ECC_DIVERSITY_USE_CRYPTOSOC

/*
 * Component: gpEncryption
 */

#define GP_ENCRYPTION_DIVERSITY_USE_AES_MMO_HW

/*
 * Component: gpFreeRTOS
 */

#define GP_DIVERSITY_FREERTOS
#define GP_FREERTOS_DIVERSITY_FORCE_USE_HEAP
#define GP_FREERTOS_DIVERSITY_HEAP
#define GP_FREERTOS_DIVERSITY_SLEEP

/*
 * Component: gphal
 */

#define GP_COMP_GPHAL_ES
#define GP_COMP_GPHAL_ES_ABS_EVENT
#define GP_COMP_GPHAL_ES_EXT_EVENT
#define GP_COMP_GPHAL_ES_EXT_EVENT_WKUP
#define GP_COMP_GPHAL_ES_REL_EVENT
#define GP_DIVERSITY_GPHAL_INTERN
#define GP_DIVERSITY_GPHAL_K8E
#define GP_DIVERSITY_GPHAL_OSCILLATOR_BENCHMARK
#define GP_DIVERSITY_GPHAL_RADIO_MGMT_SUPPORTED
#define GP_DIVERSITY_RT_SYSTEM_IN_ROM
#define GP_DIVERSITY_RT_SYSTEM_MACFILTER_IN_ROM
#define GP_DIVERSITY_RT_SYSTEM_PARTS_IN_ROM
#define GP_HAL_DIVERSITY_BLE_2MBIT_PHY_SUPPORTED
#define GP_HAL_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
#define GP_HAL_DIVERSITY_BLE_RPA
#define GP_HAL_DIVERSITY_INCLUDE_IPC
#define GP_HAL_DIVERSITY_SEC_CRYPTOSOC

/*
 * Component: gpHci
 */

#define GP_HCI_DIVERSITY_HOST_SERVER

/*
 * Component: gpJumpTables_k8e
 */

#define GP_DIVERSITY_JUMPTABLES
#define GP_DIVERSITY_JUMP_TABLE_ASSEMBLY

/*
 * Component: gpLog
 */

#define GP_LOG_DIVERSITY_NO_TIME_NO_COMPONENT_ID
#define GP_LOG_DIVERSITY_VSNPRINTF

/*
 * Component: gpNvm
 */

#define GP_DATA_SECTION_NAME_NVM                                                        gpNvm
#define GP_DATA_SECTION_START_NVM                                                       -0x4000
#define GP_NVM_DIVERSITY_ELEMENT_IF
#define GP_NVM_DIVERSITY_ELEMIF_KEYMAP
#define GP_NVM_DIVERSITY_SUBPAGEDFLASH_V2
#define GP_NVM_DIVERSITY_TAG_IF
#define GP_NVM_DIVERSITY_USE_POOLMEM
#define GP_NVM_USE_ASSERT_SAFETY_NET

/*
 * Component: gpOta
 */

#define GP_OTA_DIVERSITY_CLIENT

/*
 * Component: gpPd
 */

#define GP_DIVERSITY_PD_USE_PBM_VARIANT

/*
 * Component: gpSched
 */

#define GP_ROM_PATCHED_Sched_CanGoToSleep
#define GP_ROM_PATCHED_Sched_DumpEvent
#define GP_ROM_PATCHED_Sched_ExecEvent
#define GP_ROM_PATCHED_Sched_FindEventArg
#define GP_ROM_PATCHED_Sched_GetEvent
#define GP_ROM_PATCHED_Sched_GetEventIdlePeriod
#define GP_ROM_PATCHED_Sched_ReleaseEvent
#define GP_ROM_PATCHED_Sched_ReleaseEventBody
#define GP_ROM_PATCHED_Sched_RescheduleEvent
#define GP_ROM_PATCHED_Sched_RescheduleEventAbs
#define GP_ROM_PATCHED_Sched_ScheduleEvent
#define GP_ROM_PATCHED_Sched_ScheduleEventInSeconds
#define GP_ROM_PATCHED_gpSched_Clear
#define GP_ROM_PATCHED_gpSched_DeInit
#define GP_ROM_PATCHED_gpSched_DumpList
#define GP_ROM_PATCHED_gpSched_EventQueueEmpty
#define GP_ROM_PATCHED_gpSched_ExistsEventArg
#define GP_ROM_PATCHED_gpSched_GetRemainingTimeArgInSecAndUs
#define GP_ROM_PATCHED_gpSched_GoToSleep
#define GP_ROM_PATCHED_gpSched_Init
#define GP_ROM_PATCHED_gpSched_Main_Body
#define GP_ROM_PATCHED_gpSched_ScheduleEventArg
#define GP_ROM_PATCHED_gpSched_SetGotoSleepEnable
#define GP_ROM_PATCHED_gpSched_UnscheduleEventArg
#define GP_SCHED_DIVERSITY_SCHEDULE_INSECONDSAPI
#define GP_SCHED_DIVERSITY_SLEEP
#define GP_SCHED_DIVERSITY_USE_ARGS

/*
 * Component: gpTls
 */

#define GP_ROM_PATCHED_generate_ccm_header
#define GP_TLS_DIVERSITY_USER_DEFINED_MBEDTLS_CONFIG
#define GP_TLS_DIVERSITY_USE_MBEDTLS_ALT

/*
 * Component: gpUpgrade
 */

#define GP_APP_DIVERSITY_SECURE_BOOTLOADER
#define GP_DATA_SECTION_NAME_OTA                                                        OTA
#define GP_DATA_SECTION_SIZE_OTA                                                        0x36000
#define GP_DATA_SECTION_START_OTA                                                       -0x3a000
#define GP_DIVERSITY_APP_LICENSE_BASED_BOOT
#define GP_DIVERSITY_FLASH_APP_START_OFFSET                                             0x6000
#define GP_UPGRADE_DIVERSITY_COMPRESSION

/*
 * Component: gpUtils
 */

#define GP_DIVERSITY_UTILS_MATH
#define GP_UTILS_DIVERSITY_CIRCULAR_BUFFER
#define GP_UTILS_DIVERSITY_LINKED_LIST

/*
 * Component: halCortexM4
 */

#define GP_BSP_CONTROL_WDT_TIMER
#define GP_DIVERSITY_ENABLE_DEFAULT_BOD_HANDLING
#define GP_DIVERSITY_FLASH_BL_SIZE                                                      0x2500
#define GP_DIVERSITY_RETAIN_HEAP
#define GP_KX_FLASH_SIZE                                                                1024
#define GP_KX_HEAP_SIZE                                                                 4096
#define GP_KX_SYSRAM_SIZE                                                               32
#define GP_KX_UCRAM_SIZE                                                                96
#define HAL_DIVERSITY_UART
#define HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK
#define QPG6105

/*
 * Component: qorvoBleHost
 */

#define CORDIO_BLEHOST_DIVERSITY_HCI_INTERNAL
#define CORDIO_BLEHOST_DIVERSITY_WSF_DYNAMIC_HEAP
#define CORDIO_BLE_HOST_ATT_SERVER
#define WSF_ASSERT_ENABLED                                                              TRUE

/*
 * Component: silexCryptoSoc
 */

#define GP_ROM_PATCHED_ba414e_set_config
#define GP_SILEXCRYPTOSOC_DIVERSITY_ECC_CURVES_IN_FLASH

/*
 * Other flags
 */

#define GP_DATA_SECTION_START_JTOTA                                                     -0x3b000
#define GP_DIVERSITY_BOOTLOADER
#define GP_DIVERSITY_CORTEXM4
#define GP_DIVERSITY_LOG
#define GP_DIVERSITY_NR_OF_STACKS                                                       1
#define GP_GIT_SHA                                                                      9af6bd803b57ab2a3289042c4cff926f1ff751a0
#define GP_GIT_SHA_SHORT                                                                9af6bd8
#define GP_HAL_ES_ABS_EVENT_NMBR_OF_EVENTS                                              11
#define GP_LINKER_RESERVED_SECTIONS_PRIO_LIST                                           NVM,OTA,JTOTA
#define GP_POOLMEM_DIVERSITY_MALLOC
#define GP_ROM_PATCHED_Ble_SetAdvertiseChecker
#define HAL_DEFAULT_GOTOSLEEP_THRES                                                     1
#define HAL_DIVERSITY_PWM
#define HAL_DIVERSITY_SLEEP
#define HAL_DIVERSITY_SPI
#define HAL_DIVERSITY_TWI

#endif //_QORVO_INTERNALS_H_
