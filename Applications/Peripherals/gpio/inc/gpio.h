/*
 * Copyright (c) 2017-2021, Qorvo Inc
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
 * $Header: //depot/release/Embedded/Applications/R005_PeripheralLib/v1.3.2.1/apps/gpio/inc/gpio.h#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/**
 * @file gpio.h
 *
 * Header to map the needed GPIO definitions
 *
 */

#ifndef _GPIO_H_
#define _GPIO_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "gpBsp.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#if   defined(GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105)

/** @brief SW 3 to be used for WHITE WARM led */
#define GPIO_BUTTON_LED_1   GP_BSP_BUTTON_3
/** @brief SW 2 to be used for WHITE WARM led */
#define GPIO_BUTTON_LED_2   GP_BSP_BUTTON_2
/** @brief SW 1 to be used for RED led */
#define GPIO_BUTTON_LED_3   0
/** @brief SW 4 to be used for WHITE WARM led */
#define GPIO_BUTTON_LED_4   GP_BSP_BUTTON_4
/** @brief SW 5 to be used for WHITE WARM led */
#define GPIO_BUTTON_LED_5   GP_BSP_BUTTON_5
/** @brief SW 7 to be used for WHITE COOL led */
#define GPIO_BUTTON_LED_6   GP_BSP_BUTTON_7

#define HAL_LED_TGL_LED_1() HAL_LED_TGL_WHITE()
#define HAL_LED_TGL_LED_2() HAL_LED_TGL_WHITE_WARM()
#define HAL_LED_TGL_LED_3() HAL_LED_TGL_RED()

#define HAL_LED_INIT_LED_ALL()                                           \
    do                                                                   \
    {                                                                    \
        /*configure max threshold to overrule LED dimming in BSP files*/ \
        HAL_LED_SET_WHITE_WARM_THRESHOLD(0xFF);                           \
        HAL_LED_SET_WHITE_THRESHOLD(0xFF);                               \
        HAL_LED_SET_RED_THRESHOLD(0xFF);                                 \
    } while(false)

#define HAL_LED_SET_LED_1() HAL_LED_SET_WHITE()
#define HAL_LED_CLR_LED_1() HAL_LED_CLR_WHITE()

#define HAL_LED_SET_LED_3() HAL_LED_SET_RED()
#define HAL_LED_CLR_LED_3() HAL_LED_CLR_RED()


#else
#error Unsupported board for application
#endif
#endif //_GPIO_H_
