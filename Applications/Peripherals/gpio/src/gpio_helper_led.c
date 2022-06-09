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
 * @file gpio_helper_led.c
 *
 * API for using a GPIO as an output to drive an LED
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID        GP_COMPONENT_ID_APP

// Module include
#include "gpio.h"

// Qorvo base components
#include "gpBaseComps.h"
#include "hal.h"

// application includes
#include "gpio_poll.h"
#include "board.h"

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
 * @brief Configure the @p gpioPin to drive an LED
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 */
void gpio_LedConfigure(UInt8 gpioPin, gpio_logic_level_t logicLevel)
{
    if (logicLevel == gpio_logic_level_active_low)
    {
        /* Initialize output value to 0,
           The LED will toggle by switching between input and output */
        hal_gpioClr(gpios[gpioPin]);
        /* Install internal pull up */
        hal_gpioModePU(gpioPin, true);
    }
    else
    {
        /* Configure push pull - output */
        hal_gpioModePP(gpios[gpioPin], true);
        /* Keep pin floating */
        hal_gpioModePU(gpioPin, false);
    }

    /* Set initial state to LED OFF*/
    gpio_LedClr(gpioPin, logicLevel);
}

/*
 * @brief Modify the gpio block for @p gpioPin so the connected LED will light up
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 */
void gpio_LedSet(UInt8 gpioPin, gpio_logic_level_t logicLevel)
{
    if (logicLevel == gpio_logic_level_active_low)
    {
        /* Configure pin as output,
           This assumes the "output value" is high all the time */
        hal_gpioModePP(gpios[gpioPin], true);
    }
    else
    {
        /* Set output value high */
        hal_gpioSet(gpios[gpioPin]);
    }
}

/*
 * @brief Modify the gpio block for @p gpioPin so the connected LED will be off
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 */
void gpio_LedClr(UInt8 gpioPin, gpio_logic_level_t logicLevel)
{
    if (logicLevel == gpio_logic_level_active_low)
    {
        /* Configure pin as input,
           This stops the pin from sending current into the connected hardware and the internal pull-up resistor */
        hal_gpioModePP(gpios[gpioPin], false);
    }
    else
    {
        /* Set output value low */
        hal_gpioClr(gpios[gpioPin]);
    }
}

/*
 * @brief Returns if the LED connected to @p gpioPin is ON or not
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 *
 * @return true when the LED is ON, false when the LED is OFF
 */
Bool gpio_LedIsSet(UInt8 gpioPin, gpio_logic_level_t logicLevel)
{
    if (logicLevel == gpio_logic_level_active_low)
    {
        /* Return true if pin is configured as output */
        return hal_gpioGetModePP(gpios[gpioPin]);
    }
    else
    {
        /* Return true if output value is high */
        return hal_gpioGetSetClr(gpios[gpioPin]);
    }
}

/*
 * @brief Toggle the LED driven via @p gpioPin from ON->OFF or from OFF->ON, depending on its current state
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 */
void gpio_LedToggle(UInt8 gpioPin, gpio_logic_level_t logicLevel)
{
    if (gpio_LedIsSet(gpioPin, logicLevel))
    {
        gpio_LedClr(gpioPin, logicLevel);
    }
    else
    {
        gpio_LedSet(gpioPin, logicLevel);
    }

}
