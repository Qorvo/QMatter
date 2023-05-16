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
#define GP_COMP_BLEDATACHANNELRXQUEUE
#define GP_COMP_BLEDATACHANNELTXQUEUE
#define GP_COMP_BLEDATACOMMON
#define GP_COMP_BLEDATARX
#define GP_COMP_BLEDATATX
#define GP_COMP_BLELLCP
#define GP_COMP_BLELLCPFRAMEWORK
#define GP_COMP_BLELLCPPROCEDURES
#define GP_COMP_BLETEST
#define GP_COMP_BLETESTMODE
#define GP_COMP_COM
#define GP_COMP_GPHAL
#define GP_COMP_GPHAL_BLE
#define GP_COMP_GPHAL_MAC
#define GP_COMP_GPHAL_PBM
#define GP_COMP_GPHAL_RADIO
#define GP_COMP_GPTEST_BLE
#define GP_COMP_HALCORTEXM4
#define GP_COMP_HCI
#define GP_COMP_JUMPTABLES_K8E
#define GP_COMP_LOG
#define GP_COMP_PAD
#define GP_COMP_PD
#define GP_COMP_POOLMEM
#define GP_COMP_PTC
#define GP_COMP_RADIO
#define GP_COMP_RANDOM
#define GP_COMP_RESET
#define GP_COMP_RT_NRT_COMMON
#define GP_COMP_RXARBITER
#define GP_COMP_SCHED
#define GP_COMP_TEST
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
#define GP_COMPONENT_ID_BLETEST                                                         123
#define GP_COMPONENT_ID_BLETESTMODE                                                     230
#define GP_COMPONENT_ID_BSP                                                             8
#define GP_COMPONENT_ID_COM                                                             10
#define GP_COMPONENT_ID_GPHAL                                                           7
#define GP_COMPONENT_ID_HALCORTEXM4                                                     6
#define GP_COMPONENT_ID_HCI                                                             156
#define GP_COMPONENT_ID_JUMPTABLES_K8E                                                  60
#define GP_COMPONENT_ID_LOG                                                             11
#define GP_COMPONENT_ID_PAD                                                             126
#define GP_COMPONENT_ID_PD                                                              104
#define GP_COMPONENT_ID_POOLMEM                                                         106
#define GP_COMPONENT_ID_PTC                                                             193
#define GP_COMPONENT_ID_RADIO                                                           204
#define GP_COMPONENT_ID_RANDOM                                                          108
#define GP_COMPONENT_ID_RESET                                                           33
#define GP_COMPONENT_ID_RT_NRT_COMMON                                                   -1
#define GP_COMPONENT_ID_RXARBITER                                                       2
#define GP_COMPONENT_ID_SCHED                                                           9
#define GP_COMPONENT_ID_STAT                                                            22
#define GP_COMPONENT_ID_TEST                                                            101
#define GP_COMPONENT_ID_UTILS                                                           4
#define GP_COMPONENT_ID_VERSION                                                         129
#define GP_COMPONENT_ID_WMRK                                                            51

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
#define GP_DIVERSITY_BLE_CONNECTIONS_SUPPORTED
#define GP_DIVERSITY_BLE_DIRECTTESTMODE_SUPPORTED
#define GP_DIVERSITY_BLE_EXTENDED_REJECT_SUPPORTED
#define GP_DIVERSITY_BLE_LEGACY_ADVERTISING
#define GP_DIVERSITY_BLE_LEGACY_ADVERTISING_FEATURE_PRESENT
#define GP_DIVERSITY_BLE_PERIPHERAL
#define GP_DIVERSITY_BLE_PHY_UPDATE_SUPPORTED

/*
 * Component: gpBleDataTx
 */

#define GP_BLEDATATX_DIVERSITY_MARSHAL

/*
 * Component: gpBsp
 */

#define GP_DIVERSITY_QPG6105DK_B01

/*
 * Component: gpCom
 */

#define GP_COM_DIVERSITY_ACTIVATE_TX_CALLBACK
#define GP_COM_DIVERSITY_BLE_PROTOCOL
#define GP_COM_DIVERSITY_SERIAL_SYN

/*
 * Component: gphal
 */

#define GP_COMP_GPHAL_ES
#define GP_COMP_GPHAL_ES_ABS_EVENT
#define GP_COMP_GPHAL_ES_EXT_EVENT
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
#define GP_HAL_DIVERSITY_INCLUDE_IPC
#define GP_HAL_DIVERSITY_LEGACY_ADVERTISING
#define GP_HAL_DIVERSITY_TEST

/*
 * Component: gpHci
 */

#define GP_HCI_DIVERSITY_GPCOM_SERVER

/*
 * Component: gpJumpTables_k8e
 */

#define GP_DIVERSITY_JUMPTABLES
#define GP_DIVERSITY_JUMP_TABLE_ASSEMBLY

/*
 * Component: gpPd
 */

#define GP_DIVERSITY_PD_USE_PBM_VARIANT
#define GP_PD_DIVERSITY_MARSHAL

/*
 * Component: gpPTC
 */

#define GP_PTC_ENABLE_BLE

/*
 * Component: gpRandom
 */

#define GP_RANDOM_DIVERSITY_MARSHAL

/*
 * Component: gpRxArbiter
 */

#define GP_RXARBITER_DIVERSITY_MARSHAL

/*
 * Component: gpSched
 */

#define GP_ROM_PATCHED_Sched_CanGoToSleep
#define GP_ROM_PATCHED_gpSched_GoToSleep
#define GP_SCHED_DIVERSITY_SLEEP
#define GP_SCHED_DIVERSITY_USE_ARGS

/*
 * Component: gpTest
 */

#define GP_TEST_DIVERSITY_MAP_BLE_CW_MODES

/*
 * Component: gpUtils
 */

#define GP_DIVERSITY_UTILS_MATH
#define GP_UTILS_DIVERSITY_CIRCULAR_BUFFER
#define GP_UTILS_DIVERSITY_LINKED_LIST

/*
 * Component: gpVersion
 */

#define GP_VERSION_DIVERSITY_MARSHAL

/*
 * Component: halCortexM4
 */

#define GP_BSP_CONTROL_WDT_TIMER
#define GP_DIVERSITY_ENABLE_DEFAULT_BOD_HANDLING
#define GP_KX_FLASH_SIZE                                                                1024
#define GP_KX_SYSRAM_SIZE                                                               32
#define GP_KX_UCRAM_SIZE                                                                96
#define HAL_DIVERSITY_UART
#define QPG6105

/*
 * Other flags
 */

#define GPHAL_NUMBER_OF_PBMS_USED                                                       6
#define GP_ASSERT_DIVERSITY_MARSHAL
#define GP_BLEACTIVITYMANAGER_DIVERSITY_MARSHAL
#define GP_BLEADDRESSRESOLVER_DIVERSITY_MARSHAL
#define GP_BLEADVERTISER_DIVERSITY_MARSHAL
#define GP_BLECOMPS_DIVERSITY_MARSHAL
#define GP_BLECONFIG_DIVERSITY_MARSHAL
#define GP_BLECONNECTIONMANAGER_DIVERSITY_MARSHAL
#define GP_BLEDATACHANNELRXQUEUE_DIVERSITY_MARSHAL
#define GP_BLEDATACHANNELTXQUEUE_DIVERSITY_MARSHAL
#define GP_BLEDATACOMMON_DIVERSITY_MARSHAL
#define GP_BLEDATARX_DIVERSITY_MARSHAL
#define GP_BLEINITIATOR_DIVERSITY_MARSHAL
#define GP_BLELLCPFRAMEWORK_DIVERSITY_MARSHAL
#define GP_BLELLCPPROCEDURES_DIVERSITY_MARSHAL
#define GP_BLELLCP_DIVERSITY_MARSHAL
#define GP_BLEPRESCHED_DIVERSITY_MARSHAL
#define GP_BLESECURITYCOPROCESSOR_DIVERSITY_MARSHAL
#define GP_BLETESTMODE_DIVERSITY_MARSHAL
#define GP_BLE_DIVERSITY_MARSHAL
#define GP_BSP_DIVERSITY_MARSHAL
#define GP_COM_DIVERSITY_MARSHAL
#define GP_DIVERSITY_CORTEXM4
#define GP_DIVERSITY_LOG
#define GP_DIVERSITY_NR_OF_STACKS                                                       1
#define GP_GIT_SHA                                                                      a37dce754a22e3d021af9a733b1e2a3e0eb5c175
#define GP_GIT_SHA_SHORT                                                                a37dce7
#define GP_HAL_DIVERSITY_MARSHAL
#define GP_HAL_ES_ABS_EVENT_NMBR_OF_EVENTS                                              10
#define GP_HCI_DIVERSITY_MARSHAL
#define GP_JUMPTABLES_K8E_DIVERSITY_MARSHAL
#define GP_LOG_DIVERSITY_MARSHAL
#define GP_PAD_DIVERSITY_MARSHAL
#define GP_POOLMEM_DIVERSITY_MARSHAL
#define GP_RESET_DIVERSITY_MARSHAL
#define GP_ROM_PATCHED_Ble_SetAdvertiseChecker
#define GP_RT_NRT_COMMON_DIVERSITY_MARSHAL
#define GP_SCHED_DIVERSITY_MARSHAL
#define GP_STAT_DIVERSITY_MARSHAL
#define GP_UTILS_DIVERSITY_MARSHAL
#define GP_WMRK_DIVERSITY_MARSHAL
#define HAL_DIVERSITY_SLEEP

#endif //_QORVO_INTERNALS_H_
