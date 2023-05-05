/*
 * Copyright (c) 2017, Qorvo Inc
 *
 * hal_Mutex.h
 *   Hardware Abstraction Layer FreeRTOS sleep for ARM devices.
 *
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
 *
 */

#ifndef _HAL_SLEEPFREERTOS_H_
#define _HAL_SLEEPFREERTOS_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "global.h"
#include "FreeRTOS.h"

/*****************************************************************************
 *                    Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Function Prototype Definitions
 *****************************************************************************/
void hal_InitSleepFreeRTOS(void);
#endif //_HAL_SLEEPFREERTOS_H_
