/*
 * Copyright (c) 2017, 2020-2021, Qorvo Inc
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

/**
 * @file gpio_helper_button.c
 *
 * API for using a GPIO as an input for handling buttons
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID        GP_COMPONENT_ID_APP

// Module include
#include "gpio.h"

// Qorvo base components
#include "hal.h"

// application includes

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
/*
 * @brief Configure the @p gpioPin as a button
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the button is pressed when the input value is high (active high) or low (active high)
 */
void gpio_ButtonConfigure(UInt8 gpioPin, gpio_logic_level_t logicLevel)
{
    /* Set internal pull up when active low, set float when active high */
    hal_gpioModePU(gpioPin, logicLevel==gpio_logic_level_active_low);
    /* configure push pull - input */
    hal_gpioModePP(GPIO_PIN(gpioPin), false);
}

/** @brief Function to retrieve the output value of a @p gpioPin
 *
 *  @param[in]  gpioPin         Identifier of the button + its gpio pin onto which it is connected
 *  @param[in]  logicLevel      Indicates if the button is connected active high or active low
 *
 * @return output value of the pin
 */
UInt8 gpio_ButtonGetOutputValue(UInt8 gpioPin, gpio_logic_level_t logicLevel)
{
    if (logicLevel == gpio_logic_level_active_high)
    {
        return hal_gpioGet(GPIO_PIN(gpioPin));
    }
    else
    {
        return !hal_gpioGet(GPIO_PIN(gpioPin));
    }
}
