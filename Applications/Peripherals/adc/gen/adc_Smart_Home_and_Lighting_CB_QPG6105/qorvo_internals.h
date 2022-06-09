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
#define GP_COMP_BSP
#define GP_COMP_COM
#define GP_COMP_GPHAL
#define GP_COMP_HALCORTEXM4
#define GP_COMP_JUMPTABLES_K8E
#define GP_COMP_LOG
#define GP_COMP_RANDOM
#define GP_COMP_RESET
#define GP_COMP_SCHED
#define GP_COMP_UTILS

/*
 * Components numeric ids
 */

#define GP_COMPONENT_ID_APP                                1
#define GP_COMPONENT_ID_ASSERT                             29
#define GP_COMPONENT_ID_BASECOMPS                          35
#define GP_COMPONENT_ID_BSP                                8
#define GP_COMPONENT_ID_COM                                10
#define GP_COMPONENT_ID_GPHAL                              7
#define GP_COMPONENT_ID_HALCORTEXM4                        6
#define GP_COMPONENT_ID_JUMPTABLES_K8E                     60
#define GP_COMPONENT_ID_LOG                                11
#define GP_COMPONENT_ID_PAD                                126
#define GP_COMPONENT_ID_PD                                 104
#define GP_COMPONENT_ID_RANDOM                             108
#define GP_COMPONENT_ID_RESET                              33
#define GP_COMPONENT_ID_RXARBITER                          2
#define GP_COMPONENT_ID_SCHED                              9
#define GP_COMPONENT_ID_STAT                               22
#define GP_COMPONENT_ID_UTILS                              4

/*
 * Component: gpBsp
 */

#define GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105

/*
 * Component: gpCom
 */

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
#define GP_DIVERSITY_RT_SYSTEM_IN_ROM
#define GP_DIVERSITY_RT_SYSTEM_MACFILTER_IN_ROM
#define GP_DIVERSITY_RT_SYSTEM_PARTS_IN_ROM

/*
 * Component: gpJumpTables_k8e
 */

#define GP_DIVERSITY_JUMPTABLES
#define GP_DIVERSITY_JUMP_TABLE_ASSEMBLY

/*
 * Component: gpLog
 */

#define GP_LOG_DIVERSITY_VSNPRINTF

/*
 * Component: halCortexM4
 */

#define GP_DIVERSITY_ENABLE_DEFAULT_BOD_HANDLING
#define GP_KX_FLASH_SIZE                                   1024
#define GP_KX_RAM_SIZE                                     128
#define GP_KX_SYSRAM_SIZE                                  32
#define GP_KX_UCRAM_SIZE                                   96
#define HAL_DIVERSITY_UART
#define HAL_DIVERSITY_UART_RX_BUFFER_CALLBACK
#define QPG6105

/*
 * Other flags
 */

#define GP_BLE_NR_OF_CONNECTION_COMPLETE_EVENT_BUFFERS     0
#define GP_DIVERSITY_CORTEXM4
#define GP_DIVERSITY_LOG
#define GP_DIVERSITY_NO_BUTTON
#define GP_GIT_SHA                                         70ca0ce79609f0d40ded749fc66ef8c0e1f9f55e
#define GP_GIT_SHA_SHORT                                   70ca0ce
#define GP_HAL_ES_ABS_EVENT_NMBR_OF_EVENTS                 1

#endif //_QORVO_INTERNALS_H_
