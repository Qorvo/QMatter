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

#define GP_COMP_GPHAL_BLE
#define GP_COMP_GPHAL_MAC
#define GP_COMP_GPHAL_PBM
#define GP_COMP_GPHAL_RADIO
#define GP_COMP_GPHAL_SEC

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
#define GP_COMPONENT_ID_BLEDATACHANNELRXQUEUE                                           218
#define GP_COMPONENT_ID_BLEDATACHANNELTXQUEUE                                           219
#define GP_COMPONENT_ID_BLEDATACOMMON                                                   220
#define GP_COMPONENT_ID_BLEDATARX                                                       221
#define GP_COMPONENT_ID_BLEDATATX                                                       222
#define GP_COMPONENT_ID_BLEINITIATOR                                                    223
#define GP_COMPONENT_ID_BLELLCP                                                         224
#define GP_COMPONENT_ID_BLELLCPFRAMEWORK                                                225
#define GP_COMPONENT_ID_BLELLCPPROCEDURES                                               226
#define GP_COMPONENT_ID_BLEPRESCHED                                                     234
#define GP_COMPONENT_ID_BLESECURITYCOPROCESSOR                                          229
#define GP_COMPONENT_ID_BSP                                                             8
#define GP_COMPONENT_ID_COM                                                             10
#define GP_COMPONENT_ID_ECC                                                             192
#define GP_COMPONENT_ID_ENCRYPTION                                                      124
#define GP_COMPONENT_ID_FREERTOS                                                        24
#define GP_COMPONENT_ID_GPHAL                                                           7
#define GP_COMPONENT_ID_HALCORTEXM4                                                     6
#define GP_COMPONENT_ID_HCI                                                             156
#define GP_COMPONENT_ID_JUMPTABLESMATTER_K8E                                            60
#define GP_COMPONENT_ID_JUMPTABLES_K8E                                                  60
#define GP_COMPONENT_ID_LOG                                                             11
#define GP_COMPONENT_ID_MACCORE                                                         109
#define GP_COMPONENT_ID_MACDISPATCHER                                                   114
#define GP_COMPONENT_ID_NVM                                                             32
#define GP_COMPONENT_ID_PAD                                                             126
#define GP_COMPONENT_ID_PD                                                              104
#define GP_COMPONENT_ID_POOLMEM                                                         106
#define GP_COMPONENT_ID_QORVOBLEHOST                                                    185
#define GP_COMPONENT_ID_QVIO                                                            18
#define GP_COMPONENT_ID_RADIO                                                           204
#define GP_COMPONENT_ID_RANDOM                                                          108
#define GP_COMPONENT_ID_RESET                                                           33
#define GP_COMPONENT_ID_RT_NRT_COMMON                                                   -1
#define GP_COMPONENT_ID_RXARBITER                                                       2
#define GP_COMPONENT_ID_SCHED                                                           9
#define GP_COMPONENT_ID_SILEXCRYPTOSOC                                                  54
#define GP_COMPONENT_ID_STAT                                                            22
#define GP_COMPONENT_ID_UPGRADE                                                         115
#define GP_COMPONENT_ID_UTILS                                                           4
#define GP_COMPONENT_ID_VERSION                                                         129
#define GP_COMPONENT_ID_WMRK                                                            51

/*
 * Component: gphal
 */

#define GP_COMP_GPHAL_ES
#define GP_COMP_GPHAL_ES_ABS_EVENT
#define GP_COMP_GPHAL_ES_EXT_EVENT
#define GP_COMP_GPHAL_ES_EXT_EVENT_WKUP
#define GP_COMP_GPHAL_ES_REL_EVENT

/*
 * Component: gpNvm
 */

#define GP_DATA_SECTION_START_NVM                                                       -0x6000

/*
 * Component: gpUpgrade
 */

#define GP_DATA_SECTION_START_OTA                                                       -0x60000

/*
 * Component: halCortexM4
 */

#define GP_DIVERSITY_FLASH_BL_SIZE                                                      0x2500

/*
 * Component: uart
 */

#define CORDIO_BLEHOST_DIVERSITY_HCI_INTERNAL
#define CORDIO_BLEHOST_DIVERSITY_WSF_DYNAMIC_HEAP
#define CORDIO_BLE_HOST_ATT_SERVER
#define CORDIO_BLE_HOST_EXCLUDE_CORDIOAPPFW
#define CORDIO_BLE_HOST_EXCLUDE_SMPR
#define CORDIO_BLE_HOST_PROFILES_ORIG_SERVPROF
#define GP_APP_DIVERSITY_SECURE_BOOTLOADER
#define GP_BSP_CONTROL_WDT_TIMER
#define GP_COM_DIVERSITY_ACTIVATE_TX_CALLBACK
#define GP_COM_DIVERSITY_SERIAL_NO_SYN_NO_CRC
#define GP_COM_DIVERSITY_SERIAL_NO_SYN_SENDTO_ID                                        18
#define GP_COM_DIVERSITY_SERIAL_SYN_DISABLED
#define GP_DATA_SECTION_NAME_FACTORYDATA                                                factory_data
#define GP_DATA_SECTION_NAME_JTOTA                                                      JTOTA
#define GP_DATA_SECTION_NAME_NVM                                                        gpNvm
#define GP_DATA_SECTION_NAME_OTA                                                        OTA
#define GP_DATA_SECTION_SIZE_FACTORYDATA                                                0x800
#define GP_DATA_SECTION_SIZE_JTOTA                                                      0x1000
#define GP_DATA_SECTION_SIZE_OTA                                                        0x5A000
#define GP_DATA_SECTION_START_FACTORYDATA                                               0x4006800
#define GP_DIVERSITY_APP_LICENSE_BASED_BOOT
#define GP_DIVERSITY_BLE_ACL_CONNECTIONS_SUPPORTED
#define GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED
#define GP_DIVERSITY_BLE_LEGACY_ADVERTISING_FEATURE_PRESENT
#define GP_DIVERSITY_CORTEXM4
#define GP_DIVERSITY_ENABLE_DEFAULT_BOD_HANDLING
#define GP_DIVERSITY_FLASH_APP_START_OFFSET                                             0x8000
#define GP_DIVERSITY_FREERTOS
#define GP_DIVERSITY_GPHAL_INDIRECT_TRANSMISSION
#define GP_DIVERSITY_GPHAL_INTERN
#define GP_DIVERSITY_GPHAL_K8E
#define GP_DIVERSITY_GPHAL_OSCILLATOR_BENCHMARK
#define GP_DIVERSITY_GPHAL_RADIO_MGMT_SUPPORTED
#define GP_DIVERSITY_JUMPTABLES
#define GP_DIVERSITY_JUMP_TABLE_ASSEMBLY
#define GP_DIVERSITY_KEEP_NRT_FROM_FPGABITMAP
#define GP_DIVERSITY_NVM
#define GP_DIVERSITY_PD_USE_PBM_VARIANT
#define GP_DIVERSITY_QPG6105DK_B01
#define GP_DIVERSITY_ROMUSAGE_FOR_MATTER
#define GP_DIVERSITY_RT_SYSTEM_ADDR_FROM_LINKERSCRIPT
#define GP_DIVERSITY_RT_SYSTEM_GENERATE
#define GP_DIVERSITY_RT_SYSTEM_MACFILTER_IN_FLASH
#define GP_DIVERSITY_RT_SYSTEM_PARTS_IN_FLASH
#define GP_DIVERSITY_RT_SYSTEM_PARTS_IN_ROM
#define GP_DIVERSITY_UTILS_MATH
#define GP_ENCRYPTION_DIVERSITY_USE_AES_MMO_HW
#define GP_FREERTOS_DIVERSITY_HEAP
#define GP_FREERTOS_DIVERSITY_SLEEP
#define GP_FREERTOS_DIVERSITY_STATIC_ALLOC
#define GP_FREERTOS_DIVERSITY_USE_NON_DEFAULT_CONFIG
#define GP_FREERTOS_DIVERSITY_USE_OWN_CONFIG_HOOKS
#define GP_HAL_DIVERSITY_DUTY_CYCLE
#define GP_HAL_DIVERSITY_INCLUDE_IPC
#define GP_HAL_DIVERSITY_RAW_ENHANCED_ACK_RX
#define GP_HAL_DIVERSITY_RAW_FRAME_ENCRYPTION
#define GP_HAL_DIVERSITY_SEC_CRYPTOSOC
#define GP_HCI_DIVERSITY_HOST_SERVER
#define GP_KX_FLASH_SIZE                                                                1024
#define GP_KX_HEAP_SIZE                                                                 (4 * 1024)
#define GP_KX_SYSRAM_SIZE                                                               32
#define GP_KX_UCRAM_SIZE                                                                96
#define GP_LINKER_RESERVED_SECTIONS_PRIO_LIST                                           NVM,OTA,JTOTA
#define GP_LOG_DIVERSITY_NO_TIME_NO_COMPONENT_ID
#define GP_LOG_DIVERSITY_VSNPRINTF
#define GP_MACCORE_DIVERSITY_ASSOCIATION_ORIGINATOR
#define GP_MACCORE_DIVERSITY_ASSOCIATION_RECIPIENT
#define GP_MACCORE_DIVERSITY_FFD
#define GP_MACCORE_DIVERSITY_INDIRECT_TRANSMISSION
#define GP_MACCORE_DIVERSITY_POLL_ORIGINATOR
#define GP_MACCORE_DIVERSITY_POLL_RECIPIENT
#define GP_MACCORE_DIVERSITY_RAW_FRAMES
#define GP_MACCORE_DIVERSITY_RX_WINDOWS
#define GP_MACCORE_DIVERSITY_SCAN_ACTIVE_ORIGINATOR
#define GP_MACCORE_DIVERSITY_SCAN_ACTIVE_RECIPIENT
#define GP_MACCORE_DIVERSITY_SCAN_ED_ORIGINATOR
#define GP_MACCORE_DIVERSITY_SCAN_ORIGINATOR
#define GP_MACCORE_DIVERSITY_SCAN_ORPHAN_ORIGINATOR
#define GP_MACCORE_DIVERSITY_SCAN_ORPHAN_RECIPIENT
#define GP_MACCORE_DIVERSITY_SCAN_RECIPIENT
#define GP_MACCORE_DIVERSITY_SECURITY_ENABLED
#define GP_MACCORE_DIVERSITY_THREAD_1_2
#define GP_MACCORE_DIVERSITY_TIMEDTX
#define GP_MACDISPATCHER_DIVERSITY_SINGLE_STACK_FUNCTIONS
#define GP_NVM_DIVERSITY_ELEMENT_IF
#define GP_NVM_DIVERSITY_ELEMIF_KEYMAP
#define GP_NVM_DIVERSITY_SUBPAGEDFLASH_V2
#define GP_NVM_DIVERSITY_TAG_IF
#define GP_NVM_DIVERSITY_USE_POOLMEM
#define GP_NVM_DIVERSITY_VARIABLE_SETTINGS
#define GP_NVM_DIVERSITY_VARIABLE_SIZE
#define GP_NVM_USE_ASSERT_SAFETY_NET
#define GP_ROM_PATCHED_Ble_ClearFilterAcceptList
#define GP_ROM_PATCHED_Ble_ManipulateFilterAcceptListAllowedChecker
#define GP_ROM_PATCHED_Ble_ManipulateFilterAcceptListChecker
#define GP_ROM_PATCHED_MacCore_AnalyseMacHeader
#define GP_ROM_PATCHED_MacCore_AssociateSendCommandDataRequest
#define GP_ROM_PATCHED_MacCore_AssociateTimeout
#define GP_ROM_PATCHED_MacCore_DelayedPollConfirm
#define GP_ROM_PATCHED_MacCore_HalDataConfirm
#define GP_ROM_PATCHED_MacCore_HalDataIndication
#define GP_ROM_PATCHED_MacCore_HandleAssocConf
#define GP_ROM_PATCHED_MacCore_SendCommand
#define GP_ROM_PATCHED_MacCore_SendCommandBeaconRequest
#define GP_ROM_PATCHED_MacCore_SendCommandOrphanNotification
#define GP_ROM_PATCHED_MacCore_StopRunningRequests
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
#define GP_ROM_PATCHED_ba414e_set_config
#define GP_ROM_PATCHED_generate_ccm_header
#define GP_ROM_PATCHED_gpBleAddressResolver_EnableConnectedDevicesInFilterAcceptList
#define GP_ROM_PATCHED_gpBleAddressResolver_UpdateFilterAcceptListEntryState
#define GP_ROM_PATCHED_gpBleAddressResolver_UpdateFilterAcceptListEntryStateBulk
#define GP_ROM_PATCHED_gpBle_LeAddDeviceToFilterAcceptList
#define GP_ROM_PATCHED_gpBle_LeRemoveDeviceFromFilterAcceptList
#define GP_ROM_PATCHED_gpMacCore_AssociateRequest
#define GP_ROM_PATCHED_gpMacCore_DataRequest
#define GP_ROM_PATCHED_gpMacCore_Init
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
#define GP_RX_ARBITER_DUTY_CYCLE
#define GP_SCHED_DIVERSITY_SCHEDULE_INSECONDSAPI
#define GP_SCHED_DIVERSITY_SLEEP
#define GP_SCHED_DIVERSITY_USE_ARGS
#define GP_SILEXCRYPTOSOC_DIVERSITY_ECC_CURVES_IN_FLASH
#define GP_UPGRADE_DIVERSITY_BASE_BOOTLOADER_PATH                                       /builds/20551897/4/depot/Components/Qorvo/Bootloader/vlatest/apps/AppBootloader/UserModeBootloader_QPG6105DK_B01_matter.py
#define GP_UPGRADE_DIVERSITY_COMPRESSION
#define GP_UTILS_DIVERSITY_CIRCULAR_BUFFER
#define GP_UTILS_DIVERSITY_LINKED_LIST
#define HAL_DIVERSITY_PWM
#define HAL_DIVERSITY_SLEEP
#define HAL_DIVERSITY_TWI
#define HAL_DIVERSITY_UART
#define HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK
#define QPG6105
#define WSF_ASSERT_ENABLED                                                              TRUE

/*
 * Other flags
 */

#define GP_DATA_SECTION_START_JTOTA                                                     -0x61000
#define GP_DIVERSITY_RT_SYSTEM_IN_FLASH_VERSION                                         1
#define GP_GIT_SHA                                                                      01ee21132b9e2225f67bab20a44ad59b178d1e07
#define GP_GIT_SHA_SHORT                                                                01ee211
#define GP_HAL_ES_ABS_EVENT_NMBR_OF_EVENTS                                              0

#endif //_QORVO_INTERNALS_H_
