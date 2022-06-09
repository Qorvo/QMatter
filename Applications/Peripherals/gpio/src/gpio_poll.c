 /*
 * Copyright (c) 2022, Qorvo Inc
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

 /** @file gpio_poll.c
 *
 * Generic GPIO polling interface for the example applications.
 * This module keeps a list of registered GPIOs to track the state
 * On state change, it will inform the caller via the installed callback
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
// Module include
#include "gpio_poll.h"

// Qorvo base includes
#include "hal.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GPIO_POLL_MAX_ITEMS 6

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/
/** @struct gpio_poll_t
 *  @brief Holds the state and configuration of a monitored GPIO pin
 */
typedef struct gpio_poll_t {
    /** @brief GPIO pin to be polled */
    UInt8 gpioPin;
    /** @brief last read "output value" of the GPIO pin */
    Bool pinOutputValue;
    /** @brief Hardware configuration of the GPIO, active low/high*/
    gpio_logic_level_t logicLevel;
    /** @brief time to wait between receiving an interrupt and reading the output value */
    UInt32 debounceTimeUs;
} gpio_poll_t;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
/** @brief callback for indicating gpio change events*/
static gpio_cbPollEvent_t poll_cbPollEvent = NULL;

/** @brief number of registered inputs*/
static UInt32 pollItemCount;
/** @brief List of registered inputs*/
static gpio_poll_t pollItem[GPIO_POLL_MAX_ITEMS];

/*****************************************************************************
 *                         Static Function Prototypes
 *****************************************************************************/
static void poll_UnregisterAll(void);

/*****************************************************************************
 *                         Static Function Definitions
 *****************************************************************************/
/** @brief Unregister all inputs
 */
static void poll_UnregisterAll(void)
{
    pollItemCount = 0;
    MEMSET(&pollItem, 0x00, sizeof(pollItem));
}

/*****************************************************************************
 *                         Public Function Definitions
 *****************************************************************************/
void gpio_PollInit(gpio_cbPollEvent_t cbPollEvent)
{
    poll_cbPollEvent = cbPollEvent;
    poll_UnregisterAll();
}

void gpio_PollDeinit(void)
{
    poll_UnregisterAll();
    poll_cbPollEvent = NULL;
}

Bool gpio_PollRegisterInput(UInt8 gpioPin, UInt32 debounceTimeUs, gpio_logic_level_t logicLevel)
{
    if (pollItemCount >= GPIO_POLL_MAX_ITEMS)
    {
        /* No free slot available. */
        return false;
    }

    gpio_poll_t *p = &pollItem[pollItemCount++];

    p->gpioPin = gpioPin;
    p->debounceTimeUs = debounceTimeUs;
    p->logicLevel = logicLevel;

    return true;
}

void gpio_PollTestForChange(void)
{
    for (UInt32 i = 0; i < pollItemCount; ++i)
    {
        gpio_poll_t *p = &pollItem[i];
        UInt8 currentOutputValue = gpio_ButtonGetOutputValue(p->gpioPin, p->logicLevel);

        if(currentOutputValue != p->pinOutputValue)
        {
            if (p->debounceTimeUs)
            {
                // New output value detected, wait for debounce
                HAL_WAIT_US(p->debounceTimeUs);

                // Measure again after debounce time
                currentOutputValue = gpio_ButtonGetOutputValue(p->gpioPin, p->logicLevel);
            }

            if(currentOutputValue != p->pinOutputValue)
            {
                if (poll_cbPollEvent)
                {
                    // New output value detected, propagate state to caller
                    poll_cbPollEvent(p->gpioPin, currentOutputValue);
                }
            }

            // Store output value
            p->pinOutputValue = currentOutputValue;
        }
    }
}
