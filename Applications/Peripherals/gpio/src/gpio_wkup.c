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
 * $Header: //depot/release/Embedded/Applications/R005_PeripheralLib/v1.3.2.1/apps/gpio/src/gpio_wkup.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

 /** @file gpio_wkup.c
 *
 * GPIO example application, with 32kHz sleep mode enabled and wakeup configuration of GPIO.
 * This example shows configuring GPIO for wakeup from sleep on GPIO trigger, configuring GPIO as input
 * for Buttons and GPIO as output for LED's on a DB09-development board.
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpSched.h"
#include "gpHal.h"
#include "gpBaseComps.h"
#include "gpio.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID         GP_COMPONENT_ID_APP

/** @brief Blinking led timing set to 50ms on every 5s */
#define LED_DELAY_MS            50 //ms - 50ms
#define LED_PATTERN_PERIOD_US   5000000 //us - 5s

/** @brief Button debounce check delay in ms */
#define DELAY_DEBOUNCE_MS       10

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/** @brief Static to keep the state of all buttons
*/
static Bool Application_BtnPressed[3];

/*****************************************************************************
 *                    Application function prototypes
 *****************************************************************************/

/** @brief Function to handle GPIO changes - Button press
*/
static void Application_PollButton( void ) ;

/*****************************************************************************
 *                    Application functions
 *****************************************************************************/

/** @brief Registered Callback from Qorvo stack to signal chip wakeup
*/
void Application_cbExternalEvent(void)
{
    /* Disable interrupt untill handled */
    gpHal_EnableExternalEventCallbackInterrupt(false);

    /* Delay check for debouncing of button/signal */
    gpSched_ScheduleEvent(0, Application_PollButton);
}

/** @brief Function to handle GPIO changes - Button press
*/
void Application_PollButton(void)
{
    Bool state;

    /* Push Button for Green LED */
    state = hal_gpioGet(gpios[GPIO_BUTTON_LED_2]);
    if (!state)
    {
        if (!Application_BtnPressed[0])
        {
            /* check for debounce */
            HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
            if (!(hal_gpioGet(gpios[GPIO_BUTTON_LED_2])))
            {
                /* Button pressed, toggle green led */
                Application_BtnPressed[0] = true;
                HAL_LED_TGL_LED_2();
            }
        }
    }
    else
    {
        Application_BtnPressed[0] = false;
    }

    /* Push Button for LED 1*/
    state = hal_gpioGet(gpios[GPIO_BUTTON_LED_1]);
    if (!state)
    {
        if (!Application_BtnPressed[1])
        {
            /* check for debounce */
            HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
            if (!(hal_gpioGet(gpios[GPIO_BUTTON_LED_1])))
            {
                /* Button pressed, toggle led 1 */
                 Application_BtnPressed[1] = true;
                HAL_LED_TGL_LED_1();
            }
        }
    }
    else
    {
        Application_BtnPressed[1] = false;
    }

    /* Push Button for LED 3 - LED on while pressed */
    state = hal_gpioGet(gpios[GPIO_BUTTON_LED_3]);
    if (!state)
    {
        /* check for debounce */
        HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
        if (!(hal_gpioGet(gpios[GPIO_BUTTON_LED_3])))
        {
            /* Button pressed, enable LED 3 */
            HAL_LED_SET_LED_3();
        }
    }
    else
    {
        HAL_LED_CLR_LED_3();
    }

    /* Enable interrupt */
    gpHal_EnableExternalEventCallbackInterrupt(true);
}

/** @brief Extend initialization of GPIOs to allow wakeup from sleep on GPIO trigger
 */
void Application_InitGPIOWakeUp(void)
{
    gpHal_ExternalEventDescriptor_t eventDesc;

    /* Configure pins for wakeup */
    hal_gpioSetWakeUpMode(GPIO_BUTTON_LED_2, hal_WakeUpModeBoth);
    hal_gpioSetWakeUpMode(GPIO_BUTTON_LED_1, hal_WakeUpModeBoth);
    hal_gpioSetWakeUpMode(GPIO_BUTTON_LED_3, hal_WakeUpModeBoth);

    /* Configure External event block */
    /* Only ISR generation */
    eventDesc.type = gpHal_EventTypeDummy;
    gpHal_ScheduleExternalEvent(&eventDesc);

    /* Register handler function */
    gpHal_RegisterExternalEventCallback(Application_cbExternalEvent);

    /* Enable interrupt mask */
    gpHal_EnableExternalEventCallbackInterrupt(true);
}

/** @brief Initialize GPIO's
*/
void Application_InitGPIO(void)
{
    /* Keyboard */


    /* Button 1 - for LED 2*/
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_2, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_2], false);

    /* Button 12- for LED 1*/
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_1, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_1], false);

    /* Button 3 - for LED 3 */
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_3, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_3], false);

    /* LEDs */
    HAL_LED_INIT_LED_ALL();
}

/** @brief Generate a square waveform by toggling the IO
*/
void Application_LedPattern(void)
{
    /* Set led 2 */
    HAL_LED_TGL_LED_2();
    /* wait for delay */
    HAL_WAIT_MS(LED_DELAY_MS);
    /* clear led 2 */
    HAL_LED_TGL_LED_2();

    /* schedule event */
    gpSched_ScheduleEvent(LED_PATTERN_PERIOD_US, Application_LedPattern);
}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

 /** @brief Initialize application
 */
void Application_Init(void)
{
    /* Initialize stack */
    gpBaseComps_StackInit();

    /* initialize GPIOs used in the application */
    Application_InitGPIO();

    /* initialize GPIO to allow wake up from sleep */
    Application_InitGPIOWakeUp();

    /* Enable sleep behavior */
    gpHal_SetSleepMode(gpHal_SleepMode32kHz);

    /* Enable sleep behavior */
    gpSched_SetGotoSleepEnable(true);

    /* Schedule event led pattern event */
    gpSched_ScheduleEvent(0, Application_LedPattern);
}
