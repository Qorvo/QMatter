/*
 * Copyright (c) 2021, Qorvo Inc
 *
 * hal_Sleep.h
 *   Hardware Abstraction Layer Sleep for ARM devices.
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
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

#ifndef _HAL_SLEEP_H_
#define _HAL_SLEEP_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpAssert.h"

#ifdef GP_DIVERSITY_FREERTOS
void hal_SleepSetGotoSleepEnable(Bool enable);
void hal_SleepSetGotoSleepThreshold(TickType_t threshold);
Bool hal_SleepCheck(uint32_t xExpectedIdleTime);
#endif //GP_DIVERSITY_FREERTOS

#endif //_HAL_SLEEP_H_
