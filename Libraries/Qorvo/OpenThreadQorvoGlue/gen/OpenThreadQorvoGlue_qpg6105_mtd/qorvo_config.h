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
 * Component: gpBsp
 */

/* Contains filename of BSP header file to include */
#define GP_BSP_FILENAME                                         "gpBsp_Smart_Home_and_Lighting_CB_1_x_QPG6105.h"

/* UART baudrate */
#define GP_BSP_UART_COM_BAUDRATE                                115200


/*
 * Component: gpCom
 */

/* Multiple gpComs were specified - defined in code */
#define GP_COM_DIVERSITY_MULTIPLE_COM

/* Enable SYN datastream encapsulation */
#define GP_COM_DIVERSITY_SERIAL

/* Maximum amount of modules supported for Rx handling by gpCom. Environment already calculates minimal required module ID's */
#define GP_COM_MAX_NUMBER_OF_MODULE_IDS                         2

/* Use UART for COM - defined as default in code */
#define GP_DIVERSITY_COM_UART


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

/* Set if hal has real mutex capability */
#define HAL_MUTEX_SUPPORTED


/*
 * Component: silexCryptoSoc
 */

#define AES_GCM_EMABLED                                         0

#define AES_HW_KEYS_ENABLED                                     0

#define AES_MASK_ENABLED                                        0


#include "qorvo_internals.h"

#endif //_QORVO_CONFIG_H_
