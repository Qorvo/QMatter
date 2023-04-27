/*
 * Copyright (c) 2021, Qorvo Inc
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

 /** @file gpio_wkup.c
 *
 * GPIO example application, with external interrupt wakeup configuration.
 * Hardware setup:
 * - Buttons/Switches as GPIO input
 * - LEDs as GPIO output
 *
 * Application logic:
 * The application sleeps.
 * on external interrupt (button press / release), the application wakes up,
 * Removes the buttons as an external interrupt source to avoid triggering the
 * interrupt again while waiting for the GPIO state to settle (debounce).
 * Note, the application sleeps while waiting for the debounce timeout.
 *
 * After debouncing timeout, It reads out the state of the buttons and
 * displays this on GPIO output via LEDs.
 * The buttons are again connected as external interrupt source prior to going to
 * sleep.
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID         GP_COMPONENT_ID_APP

// Module include
#include "gpio.h"

// Qorvo base components
#include "gpBaseComps.h"
#include "gpHal.h"
#include "gpSched.h"

// Application includes
#include "gpio_poll.h"
#include "board.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
/** @brief Blinking led timing set to 50ms on every 5s */
#define APPLICATION_LED_DELAY_MS            50 //ms - 50ms
#define APPLICATION_LED_PATTERN_PERIOD_US   5000000 //us - 5s

/** @brief Time in microseconds between receiving the external interrupt and reading the GPIO output value*/
#define APPLICATION_DEBOUNCE_TIME_US        (2000)

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
static gpHal_ExternalEventCallback_t gpio_cbExternalEventOld;
static Bool Application_IsInterruptDriven = false;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
static void Application_ConfigureWakeUpEvent(void);
static void Application_cbExternalEvent(void);
static void Application_cbDebounceTimeout(void);
static void Application_SetWakeOnGpioUpdate(Bool enable);

static void Application_InitGpio(void);

static void Application_ConfigurePollingModule(void);
static void Application_cbPollEvent(UInt8 gpioPin, Bool active);
static void Application_OnButtonReleased(UInt8 gpioPin);
static void Application_OnButtonPressed(UInt8 gpioPin);

static void Application_LedPattern(void);
/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
/** @brief Initialize GPIO's */
static void Application_InitGpio(void)
{
    /* Configure output gpios as LED */
    // Deinitialize the pins that were initialized via gpBaseComps_StackInit()
    HAL_LED_DEINIT_LEDS();
    gpio_LedConfigure(GP_APP_BOARD_LED_1, GP_APP_BOARD_LED_1_LOGIC_LEVEL);
    gpio_LedConfigure(GP_APP_BOARD_LED_2, GP_APP_BOARD_LED_2_LOGIC_LEVEL);
    gpio_LedConfigure(GP_APP_BOARD_LED_3, GP_APP_BOARD_LED_3_LOGIC_LEVEL);

    /* Configure input gpios as button */
    // Deinitialize the pins that were initialized via gpBaseComps_StackInit()
    HAL_BTN_DEINIT_BTNS();

    // Configure a the GPIO peripheral for the connected buttons
    gpio_ButtonConfigure(GP_APP_BOARD_BUTTON_LED_1, GP_APP_BOARD_BUTTON_LED_1_LOGIC_LEVEL);
    gpio_ButtonConfigure(GP_APP_BOARD_BUTTON_LED_2, GP_APP_BOARD_BUTTON_LED_2_LOGIC_LEVEL);
    gpio_ButtonConfigure(GP_APP_BOARD_BUTTON_LED_3, GP_APP_BOARD_BUTTON_LED_3_LOGIC_LEVEL);
}


/** @brief Function to store which GPIOs should be tracked when calling gpio_PollTestForChange()
 */
static void Application_ConfigurePollingModule(void)
{
    // Install button event callback
    gpio_PollInit(Application_cbPollEvent);

    // Register the buttons to the polling module,
    // Set debounce time to 0 as it will be handled in external event interrupt handler
    gpio_PollRegisterInput(GP_APP_BOARD_BUTTON_LED_1, 0, GP_APP_BOARD_BUTTON_LED_1_LOGIC_LEVEL);
    gpio_PollRegisterInput(GP_APP_BOARD_BUTTON_LED_2, 0, GP_APP_BOARD_BUTTON_LED_2_LOGIC_LEVEL);
    gpio_PollRegisterInput(GP_APP_BOARD_BUTTON_LED_3, 0, GP_APP_BOARD_BUTTON_LED_3_LOGIC_LEVEL);
}

/** @brief Function to be (dis-)connect the GPIOs from the external event interrupt
 */
static void Application_SetWakeOnGpioUpdate(Bool enable)
{
    // Block new interrupts while updating which GPIOs to monitor with the external event interrupt
    gpHal_EnableExternalEventCallbackInterrupt(false);

    // Start monitoring the buttons via the external interrupt
    hal_gpioSetWakeUpMode(GP_APP_BOARD_BUTTON_LED_1, enable ? hal_WakeUpModeFalling : hal_WakeUpModeNone);
    hal_gpioSetWakeUpMode(GP_APP_BOARD_BUTTON_LED_2, enable ? hal_WakeUpModeFalling : hal_WakeUpModeNone);
    hal_gpioSetWakeUpMode(GP_APP_BOARD_BUTTON_LED_3, enable ? hal_WakeUpModeFalling : hal_WakeUpModeNone);

    // Note: Should be set prior to re-enabling the external interrupts on the buttons
    Application_IsInterruptDriven = enable;

    gpHal_EnableExternalEventCallbackInterrupt(true);

}

/** @brief Enable an interrupt on a registered external event
 *  @note This function does not configure which pins to monitor
 */
static void Application_ConfigureWakeUpEvent(void)
{
    gpHal_ExternalEventDescriptor_t eventDesc;

    // Configure External event block to trigger an interrupt
    eventDesc.type = gpHal_EventTypeDummy;
    gpHal_ScheduleExternalEvent(&eventDesc);

    /* Register handler function */
    gpio_cbExternalEventOld = gpHal_RegisterExternalEventCallback(Application_cbExternalEvent);
}

/**
 * @brief Button released event handler
 */
static void Application_OnButtonReleased(UInt8 gpioPin)
{
    /* Toggle LED on button release */
    switch (gpioPin)
    {
        case GP_APP_BOARD_BUTTON_LED_1:
            gpio_LedToggle(GP_APP_BOARD_LED_1, GP_APP_BOARD_LED_1_LOGIC_LEVEL);
            break;
        case GP_APP_BOARD_BUTTON_LED_2:
            gpio_LedToggle(GP_APP_BOARD_LED_2, GP_APP_BOARD_LED_2_LOGIC_LEVEL);
            break;
        case GP_APP_BOARD_BUTTON_LED_3:
            gpio_LedClr(GP_APP_BOARD_LED_3, GP_APP_BOARD_LED_3_LOGIC_LEVEL);
            break;
        default:
            break;
    }
}

/**
 * @brief Button pressed event handler
 */
static void Application_OnButtonPressed(UInt8 gpioPin)
{
    switch (gpioPin)
    {
        case GP_APP_BOARD_BUTTON_LED_3:
            gpio_LedSet(GP_APP_BOARD_LED_3, GP_APP_BOARD_LED_3_LOGIC_LEVEL);
            break;
        default:
            break;
    }
}

/** @brief Generate a square waveform by toggling the IO
*/
static void Application_LedPattern(void)
{
    gpio_LedToggle(GP_APP_BOARD_LED_3, GP_APP_BOARD_LED_3_LOGIC_LEVEL);
    HAL_WAIT_MS(APPLICATION_LED_DELAY_MS);
    gpio_LedToggle(GP_APP_BOARD_LED_3, GP_APP_BOARD_LED_3_LOGIC_LEVEL);
    /* schedule event */
    gpSched_ScheduleEvent(APPLICATION_LED_PATTERN_PERIOD_US, Application_LedPattern);
}

/** @brief Timeout for GPIO debouncing,
 *         Will trigger the polling module to notify any GPIO change via its installed callback
 *         Will re-enable the wake on button change for the GPIOs
 */
static void Application_cbDebounceTimeout(void)
{
    gpio_PollTestForChange();
    Application_SetWakeOnGpioUpdate(true);
}

/* CALLBACKS */

/** @brief Registered Callback from Qorvo stack to signal chip wakeup
*/
static void Application_cbExternalEvent(void)
{
    // Only handle external interrupt when the buttons are connected to the interrupt
    if (Application_IsInterruptDriven == true)
    {
        // Stop monitoring GPIOs while waiting for debounce timeout
        Application_SetWakeOnGpioUpdate(false);

        /* Delay check for debouncing of button/signal */
        /* Expect callback via the installed */
        gpSched_ScheduleEvent(APPLICATION_DEBOUNCE_TIME_US, Application_cbDebounceTimeout);
    }

    /* Chain external interrupt callbacks*/
    if (gpio_cbExternalEventOld)
    {
        gpio_cbExternalEventOld();
    }
}

/**
 * @brief Button event handler
 */
static void Application_cbPollEvent(UInt8 gpioPin, Bool active)
{
    if (active)
    {
        Application_OnButtonPressed(gpioPin);
    }
    else
    {
        Application_OnButtonReleased(gpioPin);
    }
}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/
/** @brief Initialize application
 */
void Application_Init(void)
{
    /* Initialize HAL and scheduler */
    gpBaseComps_StackInit();

    /* Initialize GPIOs used in the application */
    Application_InitGpio();

    /* Initialize helper polling module to track the state of registered GPIOs */
    Application_ConfigurePollingModule();

    /* Configure the wakeup interrupt */
    Application_ConfigureWakeUpEvent();

    /* Hook the GPIOs to external event interrupt to allow wake up from sleep */
    Application_SetWakeOnGpioUpdate(true);

    /* Enable sleep behavior */
    if (GP_BSP_32KHZ_CRYSTAL_AVAILABLE())
    {
        gpHal_SetSleepMode(gpHal_SleepMode32kHz);
    }
    else
    {
        gpHal_SetSleepMode(gpHal_SleepModeRC);
    }
    hal_SleepSetGotoSleepEnable(true);

    /* Schedule event led pattern event */
    gpSched_ScheduleEvent(0, Application_LedPattern);

    //Initialization done, application will sleep until a button update event
}
