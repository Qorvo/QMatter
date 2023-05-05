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
 */

/** @file "qvIO_IO.c"
 *
 *  IO functionality
 *
 *  Implementation of qvIO IO
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
/* <CodeGenerator Placeholder> Includes */

#define GP_COMPONENT_ID GP_COMPONENT_ID_QVIO
#define GP_MODULE_ID    GP_COMPONENT_ID

#include "qvIO.h"

#include "hal.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpBsp.h"

/* </CodeGenerator Placeholder> Includes */

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> Macro */

/** @brief Max amount of supported LEDs by current wrapper */
#define APP_MAX_LED 3

/** @brief PWM multiplier used to duty cycle LED output. */
#define PWM_DUTY_CYCLE_MULT (HAL_PWM_MAX_DUTY_CYCLE_PC / 256)

/** @brief Button debounce wait time between edge detection and polling */
#define APP_BUTTON_DEBOUNCE_PERIOD_MS 20
/** @brief Level seen as logical press. Active low buttons used hence low = pressed. */
#define APP_BUTTON_PRESSED_LEVEL 0

/** @brief Threshold to check of time of inactivity before going to sleep. (in us)*/
#ifndef APP_GOTOSLEEP_THRESHOLD
#define APP_GOTOSLEEP_THRESHOLD 1000
#endif

typedef struct IO_LedBlink_ {
    uint8_t ledNr;
    uint16_t onMs;
    uint16_t offMs;
    bool currentState;
} IO_LedBlink_t;

#ifdef GP_DIVERSITY_QPG6105DK_B01
#define GP_BSP_GPIO0_CONFIG()                                    \
    do                                                           \
    {                                                            \
        GP_WB_WRITE_IOB_GPIO_0_CFG(GP_WB_ENUM_GPIO_MODE_PULLUP); \
        GP_WB_WRITE_GPIO_GPIO0_OUTPUT_VALUE(0);                  \
        GP_WB_WRITE_GPIO_GPIO0_DIRECTION(1); /*Set as output*/   \
        HAL_LED_CLR_RED();                                       \
    } while(false)

#define GP_BSP_BUTTON_2 GP_BSP_BUTTON_GP_PB2_PIN
#define GP_BSP_BUTTON_3 GP_BSP_BUTTON_GP_PB3_PIN
#define GP_BSP_BUTTON_4 GP_BSP_BUTTON_GP_PB1_PIN
#define GP_BSP_BUTTON_5 GP_BSP_BUTTON_GP_PB4_PIN
#define GP_BSP_BUTTON_7 GP_BSP_BUTTON_GP_SW_PIN
#else
#define GP_BSP_GPIO0_CONFIG()
#endif //GP_DIVERSITY_QPG6105DK_B01

#if GP_BSP_32KHZ_CRYSTAL_AVAILABLE()
#ifndef GP_DIVERSITY_GPHAL_32KHZ_CALIBRATION_DONE_CB
#error error : GP_DIVERSITY_GPHAL_32KHZ_CALIBRATION_DONE_CB should be defined to enable external 32kHz sleep clock
#endif //GP_DIVERSITY_GPHAL_32KHZ_CALIBRATION_DONE_CB
#endif //GP_BSP_32KHZ_CRYSTAL_AVAILABLE()
/* </CodeGenerator Placeholder> Macro */

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticData */
static IO_LedBlink_t IO_LedBlinkInfo[APP_MAX_LED];

/** @brief Application callback to handle button changes */
static qvIO_pBtnCback IO_BtnCallback = NULL;
/** @brief Button state to indicate changes */
static UInt8 IO_BtnState = 0x0;
/* </CodeGenerator Placeholder> StaticData */

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

/* <CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 * --- LED handling
 *****************************************************************************/

static void LedBlink_Timeout(void* pArg)
{
    // cast void pointer to led struct - IO_LedBlink_t
    IO_LedBlink_t* ledInfo = (IO_LedBlink_t*)pArg;

    ledInfo->currentState = !ledInfo->currentState;
    if(ledInfo->currentState == false)
    {
        qvIO_LedSet(ledInfo->ledNr, false);
        gpSched_ScheduleEventArg(ledInfo->offMs * 1000, LedBlink_Timeout, (void*)ledInfo);
    }
    else
    {
        qvIO_LedSet(ledInfo->ledNr, true);
        gpSched_ScheduleEventArg(ledInfo->onMs * 1000, LedBlink_Timeout, (void*)ledInfo);
    }
}

/*****************************************************************************
 * --- Button handling
 *****************************************************************************/

static void IO_SetWakeUpMode(UInt8 mode)
{
#ifdef GP_BSP_BUTTON_1
    hal_gpioSetWakeUpMode(GP_BSP_BUTTON_1, mode);
#endif //GP_BSP_BUTTON_1
#ifdef GP_BSP_BUTTON_2
    hal_gpioSetWakeUpMode(GP_BSP_BUTTON_2, mode);
#endif //GP_BSP_BUTTON_2
#ifdef GP_BSP_BUTTON_3
    hal_gpioSetWakeUpMode(GP_BSP_BUTTON_3, mode);
#endif //GP_BSP_BUTTON_3
#ifdef GP_BSP_BUTTON_4
    hal_gpioSetWakeUpMode(GP_BSP_BUTTON_4, mode);
#endif //GP_BSP_BUTTON_4
#ifdef GP_BSP_BUTTON_5
    hal_gpioSetWakeUpMode(GP_BSP_BUTTON_5, mode);
#endif //GP_BSP_BUTTON_5
#ifdef GP_BSP_BUTTON_7
    hal_gpioSetWakeUpMode(GP_BSP_BUTTON_7, mode);
#endif //GP_BSP_BUTTON_7
}

/** @brief Retrieve GPIO status and apply to bitmask
 *
 *  @param[in]  gpioNum GPIO pin number
 *  @param[in]  btnNum Button number define
 *  @param[out] state  State bitmask variable
 *
*/
#ifdef GP_DIVERSITY_QPG6105DK_B01
#define BTN_GET_STATE(gpioNum, btnNum, state)                       \
    do                                                              \
    {                                                               \
        if(hal_gpioGet(gpios[gpioNum]) == APP_BUTTON_PRESSED_LEVEL) \
        {                                                           \
            BIT_SET(state, btnNum);                                 \
        }                                                           \
    } while(false)
#else
#define BTN_GET_STATE(gpioNum, btnNum, state)
#endif
/** @brief Retrieve GPIO status as qvIO bitmask
*
* @return Bitmask of pressed buttons, ordered in BTN_SWx order.
*
*/
UInt8 IO_GetGPIO(void)
{
    UInt8 state = 0x0;

#ifdef GP_DIVERSITY_QPG6105DK_B01
    BTN_GET_STATE(GP_BSP_BUTTON_2, BTN_SW1, state);
    BTN_GET_STATE(GP_BSP_BUTTON_7, BTN_SW2, state); // Slider switch
    BTN_GET_STATE(GP_BSP_BUTTON_3, BTN_SW3, state);
    BTN_GET_STATE(GP_BSP_BUTTON_4, BTN_SW4, state);
    BTN_GET_STATE(GP_BSP_BUTTON_5, BTN_SW5, state);
#else
#ifdef GP_BSP_BUTTON_1
    BTN_GET_STATE(GP_BSP_BUTTON_1, BTN_SW1, state);
#endif
#ifdef GP_BSP_BUTTON_2
    BTN_GET_STATE(GP_BSP_BUTTON_2, BTN_SW2, state);
#endif
#ifdef GP_BSP_BUTTON_3
    BTN_GET_STATE(GP_BSP_BUTTON_3, BTN_SW3, state);
#endif
#ifdef GP_BSP_BUTTON_4
    BTN_GET_STATE(GP_BSP_BUTTON_4, BTN_SW4, state);
#endif
#ifdef GP_BSP_BUTTON_5
    BTN_GET_STATE(GP_BSP_BUTTON_5, BTN_SW5, state);
#endif
#endif

    return state;
}

/** @brief Function to handle any GPIO changes and generate callback.
*/
static void IO_PollGPIO(void)
{
    UInt8 btnState;

    //Collect GPIO state - active low buttons
    btnState = IO_GetGPIO();

    if((IO_BtnCallback != NULL) && (IO_BtnState != btnState))
    {
        IO_BtnCallback(BTN_SW1, BIT_TST(btnState, BTN_SW1));
        IO_BtnCallback(BTN_SW2, BIT_TST(btnState, BTN_SW2));
        IO_BtnCallback(BTN_SW3, BIT_TST(btnState, BTN_SW3));
        IO_BtnCallback(BTN_SW4, BIT_TST(btnState, BTN_SW4));
        IO_BtnCallback(BTN_SW5, BIT_TST(btnState, BTN_SW5));
    }

    IO_BtnState = btnState;

    // Re-enable sensing
    IO_SetWakeUpMode(hal_WakeUpModeBoth);

#ifdef GP_DIVERSITY_QPG6105DK_B01
    gpHal_EnableExternalEventCallbackInterrupt(true);
#endif
}

/** @brief Registered Callback from Qorvo stack to signal chip wakeup
*/
static void IO_cbExternalEvent(void)
{
#ifdef GP_DIVERSITY_QPG6105DK_B01
    gpHal_EnableExternalEventCallbackInterrupt(false);
#endif

    //Remove sensing on GPIO's
    IO_SetWakeUpMode(hal_WakeUpModeNone);

    //Delay check for debouncing of button/signal
    if(!gpSched_ExistsEvent(IO_PollGPIO))
    {
        gpSched_ScheduleEvent(APP_BUTTON_DEBOUNCE_PERIOD_MS * 1000, IO_PollGPIO);
    }
}

/** @brief Extend initialization of GPIOs to allow wakeup from sleep on GPIO trigger
*/
static void IO_InitGPIOWakeUp(void)
{
    gpHal_ExternalEventDescriptor_t eventDesc;

    //-- Configure pins for wakeup
    IO_SetWakeUpMode(hal_WakeUpModeBoth);

    //Configure External event block
    eventDesc.type = gpHal_EventTypeDummy; //Only ISR generation
    gpHal_ScheduleExternalEvent(&eventDesc);

    //Register handler function
    gpHal_RegisterExternalEventCallback(IO_cbExternalEvent);

    //Enable interrupt mask
#ifdef GP_DIVERSITY_QPG6105DK_B01
    gpHal_EnableExternalEventCallbackInterrupt(true);
#endif
}

/** @brief Initialize GPIOs to use when awake
*/
static void IO_InitGPIO(void)
{
    //Init pins
    GP_BSP_GPIO0_CONFIG();

    //-- Set internal Pull-up for push buttons
#ifdef GP_BSP_BUTTON_1
    hal_gpioModePU(GP_BSP_BUTTON_1, true);
#endif //GP_BSP_BUTTON_1
#ifdef GP_BSP_BUTTON_2
    hal_gpioModePU(GP_BSP_BUTTON_2, true);
#endif //GP_BSP_BUTTON_2
#ifdef GP_BSP_BUTTON_3
    hal_gpioModePU(GP_BSP_BUTTON_3, true);
#endif //GP_BSP_BUTTON_3
#ifdef GP_BSP_BUTTON_4
    hal_gpioModePU(GP_BSP_BUTTON_4, true);
#endif //GP_BSP_BUTTON_4
#ifdef GP_BSP_BUTTON_5
    hal_gpioModePU(GP_BSP_BUTTON_5, true);
#endif //GP_BSP_BUTTON_5
#ifdef GP_BSP_BUTTON_7
    hal_gpioModePU(GP_BSP_BUTTON_7, true);
#endif //GP_BSP_BUTTON_7
}
/* </CodeGenerator Placeholder> StaticFunctionDefinitions */

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/

/** @brief Initialize qvIO interface for use.
 *
*/
void qvIO_Init(void)
{
    IO_InitGPIO();
    IO_InitGPIOWakeUp();
#ifdef GP_BSP_PWM_GPIO_MAP
    //Init PWM
    hal_InitPWM();

    hal_SetChannelEnabled(PWM_CHANNEL_RED, false);
    hal_SetChannelEnabled(PWM_CHANNEL_GREEN, false);
    hal_SetChannelEnabled(PWM_CHANNEL_BLUE, false);
    hal_SetChannelEnabled(PWM_CHANNEL_WHITE_COOL, false);
    hal_SetChannelEnabled(PWM_CHANNEL_WHITE_WARM, false);
#endif //GP_BSP_PWM_GPIO_MAP
}

/*****************************************************************************
 * LED control
 *****************************************************************************/

/** @brief Set LED ON or OFF.
*
*   @param ledNr                     The index of the LED that is controlled.
*   @param state                     LED on (true) or off (false).
*/
bool qvIO_LedSet(uint8_t ledNr, bool state)
{
    /* <CodeGenerator Placeholder> Implementation_qvIO_LedSet */
    IO_LedBlink_t* localLedInfo;

    if(ledNr >= APP_MAX_LED)
    {
        return false;
    }

    /* if a blinking for this LED exists, stop it */
    localLedInfo = &IO_LedBlinkInfo[ledNr];
    if(gpSched_ExistsEventArg(LedBlink_Timeout, (void*)localLedInfo))
    {
        gpSched_UnscheduleEventArg(LedBlink_Timeout, (void*)localLedInfo);
        /* reset structure */
        MEMSET(localLedInfo, 0x00, sizeof(IO_LedBlink_t));
    }

    if(state == false)
    {
        if(ledNr == LED_WHITE)
        {
            qvIO_PWMSetLevel(PWM_CHANNEL_WHITE_COOL, 0);
#ifdef GP_BSP_PWM_GPIO_MAP
            hal_SetChannelEnabled(PWM_CHANNEL_WHITE_COOL, false);
#endif
        }
        else if(ledNr == LED_GREEN)
        {
            HAL_LED_CLR_GRN();
        }
        else if(ledNr == LED_RED)
        {
            HAL_LED_CLR_RED();
        }
    }
    else
    {
        if(ledNr == LED_WHITE)
        {
#ifdef GP_BSP_PWM_GPIO_MAP
            hal_SetChannelEnabled(PWM_CHANNEL_WHITE_COOL, true);
#endif
            qvIO_PWMSetLevel(PWM_CHANNEL_WHITE_COOL, 255);
        }
        else if(ledNr == LED_GREEN)
        {
            HAL_LED_SET_GRN();
        }
        else if(ledNr == LED_RED)
        {
            HAL_LED_SET_RED();
        }
    }

    return true;
    /* </CodeGenerator Placeholder> Implementation_qvIO_LedSet */
}

/** @brief Blink a LED with specified on and off period.
*
*   @param ledNr                     The index of the LED that is controlled.
*   @param onMs                      How many msec should the ON state last.
*   @param offMs                     How many msec should the OFF state last.
*/
bool qvIO_LedBlink(uint8_t ledNr, uint16_t onMs, uint16_t offMs)
{
    /* <CodeGenerator Placeholder> Implementation_qvIO_LedBlink */
    if(ledNr < APP_MAX_LED)
    {
        /* store parameters in the local blinking info structure */
        IO_LedBlink_t* localLedInfo;

        localLedInfo = &IO_LedBlinkInfo[ledNr];

        /* only modify blinking if something is changed */
        if((localLedInfo->onMs != onMs) || (localLedInfo->offMs != offMs))
        {
            localLedInfo->ledNr = ledNr;
            localLedInfo->onMs = onMs;
            localLedInfo->offMs = offMs;
            localLedInfo->currentState = true;

            /* stop blinking for current LED if in progress */
            gpSched_UnscheduleEventArg(LedBlink_Timeout, (void*)localLedInfo);

            qvIO_LedSet(ledNr, true);
            gpSched_ScheduleEventArg(localLedInfo->onMs * 1000, LedBlink_Timeout, (void*)localLedInfo);
        }

        return true;
    }

    return false;
    /* </CodeGenerator Placeholder> Implementation_qvIO_LedBlink */
}

/*****************************************************************************
 * Button control
 *****************************************************************************/

/** @brief Store internally an upper layer callback for signaling button presses.
*
*   @param btnCback                  Pointer to the button handler to be stored internally.
*/
void qvIO_SetBtnCallback(qvIO_pBtnCback btnCback)
{
    /* <CodeGenerator Placeholder> Implementation_qvIO_SetBtnCallback */
    IO_BtnCallback = btnCback;
    /* </CodeGenerator Placeholder> Implementation_qvIO_SetBtnCallback */
}

/*****************************************************************************
 * PWM control
 *****************************************************************************/

/** @brief turns color LED on or off
*
*   @param onoff              true for on, false for off
*/
void qvIO_PWMColorOnOff(bool onoff)
{
#ifdef GP_BSP_PWM_GPIO_MAP
    /* <CodeGenerator Placeholder> Implementation_qvIO_PWMColorOnOff */
    if(onoff)
    {
        // Enable pwm channels for RGB LED
        hal_SetChannelEnabled(PWM_CHANNEL_RED, true);
        hal_SetChannelEnabled(PWM_CHANNEL_GREEN, true);
        hal_SetChannelEnabled(PWM_CHANNEL_BLUE, true);

        hal_EnablePwm(true);
    }
    else
    {
        // Disable pwm channels for RGB LED
        hal_EnablePwm(false);

        hal_SetChannelEnabled(PWM_CHANNEL_RED, false);
        hal_SetChannelEnabled(PWM_CHANNEL_GREEN, false);
        hal_SetChannelEnabled(PWM_CHANNEL_BLUE, false);
    }
    /* </CodeGenerator Placeholder> Implementation_qvIO_PWMColorOnOff */
#endif //GP_BSP_PWM_GPIO_MAP
}

/** @brief sets RGB color of led 255 == 100%
*
*   @param r                    intensity of red (0-255)
*   @param g                    intensity of green (0-255)
*   @param b                    intensity of blue (0-255)
*/
void qvIO_PWMSetColor(uint8_t r, uint8_t g, uint8_t b)
{
    /* <CodeGenerator Placeholder> Implementation_qvIO_PWMSetColor */
#ifdef GP_BSP_PWM_GPIO_MAP
    // map onto 1 - 10000 range
    hal_SetDutyCyclePercentage(PWM_CHANNEL_RED, (UInt32)r * PWM_DUTY_CYCLE_MULT);
    hal_SetDutyCyclePercentage(PWM_CHANNEL_GREEN, (UInt32)g * PWM_DUTY_CYCLE_MULT);
    hal_SetDutyCyclePercentage(PWM_CHANNEL_BLUE, (UInt32)b * PWM_DUTY_CYCLE_MULT);

#endif //GP_BSP_PWM_GPIO_MAP
    /* </CodeGenerator Placeholder> Implementation_qvIO_PWMSetColor */
}

/** @brief sets brightness of led 255 = 100%
*
*   @param channel              pwm channel
*   @param level                intensity of level (0-255)
*/
void qvIO_PWMSetLevel(uint8_t channel, uint8_t level)
{
#ifdef GP_BSP_PWM_GPIO_MAP
    hal_SetDutyCyclePercentage(channel, (UInt32)level * PWM_DUTY_CYCLE_MULT);
#endif //GP_BSP_PWM_GPIO_MAP
}

/*****************************************************************************
 * Sleep mode control
 *****************************************************************************/

void qvIO_EnableSleep(bool enable)
{
#if defined(GP_DIVERSITY_GPHAL_K8E)
/**
* Porting note:
*
* Keeping flash component in active state under high-temperature environments (>85 degrees Celsius)
* has a negative impact on flash wear which has a direct correlation to the product lifetime.
* Therefor it is recommended to enable sleep in all high temperature applications with the API
* mentioned below.
*
**/
#endif // defined(GP_DIVERSITY_GPHAL_K8A) || defined(GP_DIVERSITY_GPHAL_K8C) || defined(GP_DIVERSITY_GPHAL_K8E)

    // Calling the API below will enable the ARM processor to enter a low-power mode.
    // IC peripherals, such as but not limited to the receiver, GPIO, LED block, PWM, etc.
    // will remain awake if the application requires their functionality even with this API
    // called and the ARM in sleep mode.

    if(enable)
    {
        //prevent enabling sleep mode twice, it will cause an assertion otherwise
        hal_SleepSetGotoSleepEnable(false);

        if(!GP_BSP_32KHZ_CRYSTAL_AVAILABLE())
        {
            /* Select internal 32kHz RC oscillator */
            gpHal_SetSleepMode(gpHal_SleepModeRC);
        }
        hal_SleepSetGotoSleepThreshold(APP_GOTOSLEEP_THRESHOLD);
    }
    hal_SleepSetGotoSleepEnable(enable);
}

#ifdef GP_DIVERSITY_GPHAL_32KHZ_CALIBRATION_DONE_CB
void gpHal_cb32kHzCalibrationDone(gpHal_SleepClockMeasurementStatus_t status, UInt32 mse)
{
    if(GP_BSP_32KHZ_CRYSTAL_AVAILABLE() && status == gpHal_SleepClockMeasurementStatusStable)
    {
        gpHal_SetSleepMode(gpHal_SleepMode32kHz);
    }
    else
    {
        // Fall back to RC sleep mode if 32kHz cannot be calibrated.
        gpHal_SetSleepMode(gpHal_SleepModeRC);
    }
}
#endif // defined(GP_DIVERSITY_GPHAL_32KHZ_CALIBRATION_DONE_CB)
