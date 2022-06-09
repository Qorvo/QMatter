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
 * $Header$
 * $Change$
 * $DateTime$
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
#include "global.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/
/** @enum gpio_logic_level_t */
//@{
#define gpio_logic_level_active_low     0
#define gpio_logic_level_active_high    1
/** @typedef gpio_logic_level_t
 *  @brief List of logical levels for a button
*/
typedef UInt8                             gpio_logic_level_t;
//@}

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/
/* LED */
/*
 * @brief Configure the @p gpioPin to drive an LED
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 */
void gpio_LedConfigure(UInt8 gpioPin, gpio_logic_level_t logicLevel);

/*
 * @brief Modify the gpio block for @p gpioPin so the connected LED will light up
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 */
void gpio_LedSet(UInt8 gpioPin, gpio_logic_level_t logicLevel);

/*
 * @brief Modify the gpio block for @p gpioPin so the connected LED will be off
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 */
void gpio_LedClr(UInt8 gpioPin, gpio_logic_level_t logicLevel);

/*
 * @brief Returns if the LED connected to @p gpioPin is ON or not
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 *
 * @return true when the LED is ON, false when the LED is OFF
 */
Bool gpio_LedIsSet(UInt8 gpioPin, gpio_logic_level_t logicLevel);

/*
 * @brief Toggle the LED driven via @p gpioPin from ON->OFF or from OFF->ON, depending on its current state
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the LED is ON when the ouput is high (active high) or low (active low)
 */
void gpio_LedToggle(UInt8 gpioPin, gpio_logic_level_t logicLevel);

/* BUTTON */
/*
 * @brief Configure the @p gpioPin as a button
 *
 * @param gpioPin    The GPIO pin on which the LED is connected
 * @param logicLevel Indicates if the button is pressed when the input value is high (active high) or low (active high)
 */
void gpio_ButtonConfigure(UInt8 gpioPin, gpio_logic_level_t logicLevel);

/** @brief Function to retrieve the output value of a @p gpioPin
 *
 *  @param[in]  gpioPin         Identifier of the button + its gpio pin onto which it is connected
 *  @param[in]  logicLevel      Indicates if the button is connected active high or active low
 *
 * @return output value of the pin
 */
UInt8 gpio_ButtonGetOutputValue(UInt8 gpioPin, gpio_logic_level_t logicLevel);

#endif //_GPIO_H_
