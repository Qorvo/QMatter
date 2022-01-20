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
 * $Header: //depot/release/Embedded/Applications/R005_PeripheralLib/v1.3.2.1/apps/gpio/src/gpio.c#1 $
 * $Change: 189026 $
 * $DateTime: 2022/01/18 14:46:53 $
 *
 */

/**
 * @file gpio.c
 *
 * GPIO example application, without sleep and non-wakeup configuration of GPIO.
 * This application shows an example of configuring GPIO as input for Buttons and GPIO as output for LED's.

 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpio.h"
#include "gpHal.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID        GP_COMPONENT_ID_APP

/** @brief Button debounce check delay in ms */
#define DELAY_DEBOUNCE_MS      10

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/** @brief Static to keep the state of all buttons
*/
#ifdef GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105
//QPG6105 Development board has 5 buttons to control
static Bool Application_BtnPressed[4];
#else //GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105
static Bool Application_BtnPressed[3];
#endif //GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105

/*****************************************************************************
 *                    Application functions
 *****************************************************************************/

/** @brief Function to handle GPIO changes - Button press
*/
void Application_PollButton(void)
{
    Bool state;

    /* Push Button for led 2*/
    state = hal_gpioGet(gpios[GPIO_BUTTON_LED_2]);
    if (!state)
    {
        if (!Application_BtnPressed[0])
        {
            /* check for debounce */
            HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
            if (!(hal_gpioGet(gpios[GPIO_BUTTON_LED_2])))
            {
                /* Button pressed, toggle led 2 */
                Application_BtnPressed[0] = true;
                HAL_LED_TGL_LED_2();
            }
        }
    }
    else
    {
        Application_BtnPressed[0] = false;
    }

    /* Push Button for LED 1 */
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
#ifdef GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105
                //For QPG6105 Development board, all the buttons need to control LED_2
                HAL_LED_TGL_LED_2();
#else //GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105
                HAL_LED_TGL_LED_1();
#endif //GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105
            }
        }
    }
    else
    {
        Application_BtnPressed[1] = false;
    }

#if defined(GPIO_BUTTON_LED_4)
    /* Push Button for Green LED */
    state = hal_gpioGet(gpios[GPIO_BUTTON_LED_4]);
    if (!state)
    {
        if (!Application_BtnPressed[2])
        {
            /* check for debounce */
            HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
            if (!(hal_gpioGet(gpios[GPIO_BUTTON_LED_4])))
            {
                /* Button pressed, toggle green led */
                Application_BtnPressed[2] = true;
                HAL_LED_TGL_LED_2();
            }
        }
    }
    else
    {
        Application_BtnPressed[2] = false;
    }
#endif

#if defined(GPIO_BUTTON_LED_5)
    /* Push Button for Green LED */
    state = hal_gpioGet(gpios[GPIO_BUTTON_LED_5]);
    if (!state)
    {
        if (!Application_BtnPressed[3])
        {
            /* check for debounce */
            HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
            if (!(hal_gpioGet(gpios[GPIO_BUTTON_LED_5])))
            {
                /* Button pressed, toggle green led */
                Application_BtnPressed[3] = true;
                HAL_LED_TGL_LED_2();
            }
        }
    }
    else
    {
        Application_BtnPressed[3] = false;
    }
#endif


#if defined(GPIO_BUTTON_LED_6)
    /* Push Button for Green LED */
    state = hal_gpioGet(gpios[GPIO_BUTTON_LED_6]);
    if (!state)
    {
        /* check for debounce */
        HAL_WAIT_MS(DELAY_DEBOUNCE_MS);
        if (!(hal_gpioGet(gpios[GPIO_BUTTON_LED_6])))
        {
            /* Button pressed, enable LED 3 */
            /* configure push pull - input */
            HAL_LED_SET_LED_1();
        }
    }
    else
    {
        HAL_LED_CLR_LED_1();
    }
#endif

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
}

/** @brief Initialize GPIO's */
void Application_InitGPIO(void)
{
    /* Keyboard */


#if defined(GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105)
    //In this application GPIO0 needs to be used as a button
    GP_WB_WRITE_IOB_GPIO_0_ALTERNATE_ENABLE(0);
#endif // GP_DIVERSITY_SMART_HOME_AND_LIGHTING_CB_QPG6105
    /* Button 1 - for LED 2 */
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_2, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_2], false);

    /* Button 2 - for LED 1*/
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_1, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_1], false);

#if defined(GPIO_BUTTON_LED_4)
    /* Button 4 - for LED 2 */
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_4, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_4], false);
#endif

    /* Button 3 - for LED 3 */
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_3, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_3], false);

#if defined(GPIO_BUTTON_LED_5)
    /* Button 4 - for LED 2 */
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_5, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_5], false);
#endif

#if defined(GPIO_BUTTON_LED_6)
    /* Button 4 - for LED 2 */
    /* Set internal pull up */
    hal_gpioModePU(GPIO_BUTTON_LED_6, true);
    /* configure push pull - input */
    hal_gpioModePP(gpios[GPIO_BUTTON_LED_6], false);
#endif

    /* LEDs */
    HAL_LED_INIT_LED_ALL();

}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

/** @brief Initialize application */
void Application_Init(void)
{
    HAL_INIT();
#if !defined(HAL_DIVERSITY_WDT_DISABLE)
    HAL_WAIT_MS(1000);
#endif
    HAL_WDT_DISABLE();

    Application_InitGPIO();
    MEMSET(Application_BtnPressed, false, sizeof(Application_BtnPressed));
}

/*****************************************************************************
 *                    Main function
 *****************************************************************************/

#ifdef GP_SCHED_EXTERNAL_MAIN
/** @brief Main function */
MAIN_FUNCTION_RETURN_TYPE MAIN_FUNCTION_NAME(void)
{
    /* Intialize application */
    Application_Init();

    while (true)
    {
        /* Check input button */
        Application_PollButton();
    }
}
#endif
