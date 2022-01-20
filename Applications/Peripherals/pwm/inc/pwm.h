/*
 * Copyright (c) 2017, Qorvo Inc
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
 * $Header: //depot/release/Embedded/Applications/R005_PeripheralLib/v1.3.2.1/apps/pwm/inc/pwm.h#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/**
 * @file pwm.h
 *
 * Header to map the needed GPIO definitions
 *
 */

#ifndef _PWM_H_
#define _PWM_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBsp.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#if   defined(GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105)

#define PWM_GPIO_RED   0
#define PWM_GPIO_GREEN 1
#define PWM_GPIO_BLUE  2

#else
#error Unsupported board for application

#endif

#endif //_PWM_H_
