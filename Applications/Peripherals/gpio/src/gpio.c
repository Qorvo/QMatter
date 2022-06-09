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
 * @file gpio.c
 *
 * GPIO example application
 * This application shows an example of configuring GPIO as input for Buttons and GPIO as output for LED's.
 * It uses the gpio_button_poll.c module to monitor the state of the buttons.
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
/** @brief Time in microseconds between receiving the external interrupt and reading the GPIO output value*/
#define APPLICATION_DEBOUNCE_TIME_US    (2000)

#if defined(GP_DIVERSITY_FREERTOS)
/** @brief Priority of the main task*/
#define mainQUEUE_RECEIVE_TASK_PRIORITY        ( tskIDLE_PRIORITY + 2 )
#endif

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/
#if defined(GP_DIVERSITY_FREERTOS)
static StaticTask_t xPollTaskPCB;
static StackType_t xPollTaskStack[configMINIMAL_STACK_SIZE];
#endif

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/
#if defined(GP_SCHED_EXTERNAL_MAIN) || defined(GP_DIVERSITY_FREERTOS)
static void Application_MainLoop(void *pvParameters);
#endif

static void Application_InitGPIO(void);

static void Application_cbPollEvent(UInt8 gpioPin, Bool active);
static void Application_OnButtonReleased(UInt8 gpioPin);
static void Application_OnButtonPressed(UInt8 gpioPin);

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/
/** @brief Initialize GPIO's */
static void Application_InitGPIO(void)
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
    gpio_PollRegisterInput(GP_APP_BOARD_BUTTON_LED_1, APPLICATION_DEBOUNCE_TIME_US, GP_APP_BOARD_BUTTON_LED_1_LOGIC_LEVEL);
    gpio_PollRegisterInput(GP_APP_BOARD_BUTTON_LED_2, APPLICATION_DEBOUNCE_TIME_US, GP_APP_BOARD_BUTTON_LED_2_LOGIC_LEVEL);
    gpio_PollRegisterInput(GP_APP_BOARD_BUTTON_LED_3, APPLICATION_DEBOUNCE_TIME_US, GP_APP_BOARD_BUTTON_LED_3_LOGIC_LEVEL);
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

/* CALLBACKS */
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

/** @brief Initialize application */
void Application_Init(void)
{
    gpBaseComps_StackInit();

    /* Initialize GPIOs used in the application */
    Application_InitGPIO();

    /* Initialize helper polling module to track the state of registered GPIOs */
    Application_ConfigurePollingModule();

#if defined(GP_DIVERSITY_FREERTOS)
    TaskHandle_t TaskHandle;

    TaskHandle = xTaskCreateStatic(
            Application_MainLoop,                   /* The function that implements the task. */
            "buttonpoll",                           /* The text name assigned to the task - for debug only as it is not used by the kernel. */
            configMINIMAL_STACK_SIZE,               /* The size of the stack to allocate to the task. */
            NULL,                                   /* The parameter passed to the task */
            mainQUEUE_RECEIVE_TASK_PRIORITY,        /* The priority assigned to the task. */
            xPollTaskStack,                         /* The task stack memory */
            &xPollTaskPCB);                         /* The task PCB memory */
    GP_ASSERT (GP_DIVERSITY_ASSERT_LEVEL_SYSTEM, TaskHandle!=NULL);
#endif
}

/*****************************************************************************
 *                    Main function
 *****************************************************************************/

#if defined(GP_SCHED_EXTERNAL_MAIN) || defined(GP_DIVERSITY_FREERTOS)
static void Application_MainLoop(void *pvParameters)
{
    NOT_USED(pvParameters);

    // Disable the watchdog prior to entering the blocking polling loop.
    HAL_WDT_DISABLE();

    while (true)
    {
        /* Check input events */
        gpio_PollTestForChange();
    }
}
#endif

#if defined(GP_SCHED_EXTERNAL_MAIN) && !defined(GP_DIVERSITY_FREERTOS)
/** @brief Main function */
MAIN_FUNCTION_RETURN_TYPE MAIN_FUNCTION_NAME(void)
{
    HAL_INIT();
    /* Intialize application */
    Application_Init();
    Application_MainLoop(NULL);

    /* Should never reach this point */
}
#endif
