/*
 * Copyright (c) 2022, Qorvo Inc
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


/** @file "gpio_poll.h"
 *
 *  Polling Interface
 *
*/

#ifndef _GPIO_POLL_H_
#define _GPIO_POLL_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#include "global.h"
#include "gpio.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
/* @brief Prototype for a GPIO output value update callback
 *
 * @param gpioPin  the GPIO pin that has a state change
 * @param active   Indicates if the output value is active or not (based on the configured logic level)
 */
typedef void (*gpio_cbPollEvent_t) (UInt8 gpioPin, Bool active);

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/
/** @brief Initialize the polling module
 *
 * @param cbPollEvent   callback to handle output value change events
 */
void gpio_PollInit(gpio_cbPollEvent_t cbPollEvent);

/** @brief Deinitialize the polling module, this will unregister all inputs
 */
void gpio_PollDeinit(void);


/** @brief Register a GPIO to the polling module
 *
 *  @param[in]  gpioPin         Identifier of the button + its gpio pin onto which it is connected
 *  @param[in]  debounceTimeUs  Time to wait between detection of GPIO output value change and measuring the value
 *  @param[in]  logicLevel      Indicates if the button is connected active high or active low
 *
 * @return false when no free slots were available to register
 *         true when registered successfully
 */
Bool gpio_PollRegisterInput(UInt8 gpioPin, UInt32 debounceTimeUs, gpio_logic_level_t logicLevel);

/** @brief Poll all registered inputs,
 *         Alert the caller via the installed cbPollEvent on change
 *         On a detected ouput value change, the function will wait for the pin specific debounce time
 *         It will remeasure the output value to verify the change, prior to alerting the caller
 */
void gpio_PollTestForChange(void);

#endif //_GPIO_POLL_H_

