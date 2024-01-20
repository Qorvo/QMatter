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

#define GP_CHANGELIST                                           0


/*
 * Component: gpAssert
 */

/* Choose reset as default assert action */
#define GP_DIVERSITY_ASSERT_ACTION_RESET

/* Choose to do nothing as default assert reporting */
#define GP_DIVERSITY_ASSERT_REPORTING_NOTHING


/*
 * Component: gpBsp
 */

/* Contains filename of BSP header file to include */
#define GP_BSP_FILENAME                                         "gpBsp_QPG6105DK_B01.h"

/* UART baudrate */
#define GP_BSP_UART_COM_BAUDRATE                                115200


/*
 * Component: gpCom
 */

/* Multiple gpComs were specified - defined in code */
#define GP_COM_DIVERSITY_MULTIPLE_COM

/* Enable SYN datastream encapsulation */
#define GP_COM_DIVERSITY_SERIAL

/* Use UART for COM - defined as default in code */
#define GP_DIVERSITY_COM_UART


/*
 * Component: gphal
 */

/* Amount of 64-bit long IEEE addresses entries to keep data pending for */
#define GPHAL_DP_LONG_LIST_MAX                                  10

/* Amount of 16-bit short address entries to keep data pending for */
#define GPHAL_DP_SHORT_LIST_MAX                                 10

/* minimum required RT system version */
#define GPHAL_MIN_RTSYSTEMVERSION                               0


/*
 * Component: gpJumpTables_k8e
 */

/* gp scheduler ROM ver 2.0 */
#define GP_DIVERSITY_ROM_GPSCHED_V2


/*
 * Component: gpMacCore
 */

/* Number of known Neighbours for use with indirect transmission */
#define GP_MACCORE_MAX_NEIGHBOURS                               10


/*
 * Component: gpNvm
 */

/* Maximum number of unique tags in each pool. Used for memory allocation at Tag level API */
#define GP_NVM_NBR_OF_UNIQUE_TAGS                               32

#define GP_NVM_TYPE                                             6


/*
 * Component: gpSched
 */

/* Callback after every main loop iteration. */
#define GP_SCHED_NR_OF_IDLE_CALLBACKS                           0


/*
 * Component: halCortexM4
 */

/* Maximum size reserved for RT in flash. */
#define GP_DIVERSITY_RT_SYSTEM_MAX_FLASH_SIZE                   0xE000

/* Set if hal has real mutex capability. Used to skip even disabling/enabling global interrupts. */
#define HAL_MUTEX_SUPPORTED


/*
 * Component: silexCryptoSoc
 */

#define AES_GCM_EMABLED                                         0

#define AES_HW_KEYS_ENABLED                                     0

#define AES_MASK_ENABLED                                        0


#include "qorvo_internals.h"

#endif //_QORVO_CONFIG_H_
