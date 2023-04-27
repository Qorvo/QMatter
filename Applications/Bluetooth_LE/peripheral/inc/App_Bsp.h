/*
 *   Copyright (c) 2018, GreenPeak Technologies
 *   Copyright (c) 2018, Qorvo Inc
 *
 *   Application API
 *   Declarations of the public functions and enumerations of App.
 *
 *   This software is owned by Qorvo Inc
 *   and protected under applicable copyright laws.
 *   It is delivered under the terms of the license
 *   and is intended and supplied for use solely and
 *   exclusively with products manufactured by
 *   Qorvo Inc.
 *   
 *   
 *   THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 *   CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 *   IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 *   LIMITED TO, IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *   QORVO INC. SHALL NOT, IN ANY
 *   CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 *   INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 *   FOR ANY REASON WHATSOEVER.
 *   
 *   $Header$
 *   $Change$
 *   $DateTime$
 */


#ifndef _APP_BSP_H_
#define _APP_BSP_H_

/**
 * @file AppBsp.h.h
 *
 * Header to map the needed GPIO definitions
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "global.h"
#include "gpBsp.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define APP_BSP_GPIO_SLIDER         GP_SW
#define APP_BLE_START_BUTTON        GP_PB4
#define APP_BLE_STOP_BUTTON         GP_PB3

#endif //_APP_BSP_H_

