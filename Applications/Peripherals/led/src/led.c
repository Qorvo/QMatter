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
 * $Header$
 * $Change$
 * $DateTime$
 *
 */

/** @file led.c
 *
 * LED example application.
 * This example control LED light.
 *
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpCom.h"
#include "gpLog.h"
#include "gpReset.h"
#include "gpBaseComps.h"
#include "gpBsp.h"


/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

#if   \
    defined(GP_DIVERSITY_QPG6105DK_B01)
#ifndef WHITE_COOL
    #define HAL_LED_INIT_WHITE_COOL()                         do{ \
        /*We need to use GPIO17 for LED so disable PWM channel.*/ \
        GP_BSP_PWM4_DEINIT(); \
        /*Initialize LED driver block 2 for GPIO 17*/ \
        GP_WB_WRITE_LEDS_LED2_THRESHOLD(HAL_LED_WHITE_COOL_MAX_OUTPUT); \
        GP_WB_WRITE_IOB_GPIO_17_ALTERNATE(GP_WB_ENUM_GPIO_17_ALTERNATES_LED_DO_2); \
        GP_WB_WRITE_IOB_GPIO_17_ALTERNATE_ENABLE(1); \
        HAL_LED_CLR_WHITE_COOL(); \
        /*Initialize timer for LED PWM function*/ \
        GP_WB_WRITE_TIMERS_TMR2_PRESCALER_DIV(7); \
        GP_WB_WRITE_TIMERS_TMR2_THRESHOLD(255); \
        GP_WB_WRITE_TIMERS_TMR_PRESET_VALUE(0); \
        GP_WB_TIMERS_TMR2_PRESET(); \
        GP_WB_WRITE_TIMERS_TMR2_ENABLE(1); \
        GP_WB_WRITE_LEDS_MAIN_TMR(GP_WB_ENUM_TMR_SEL_TMR2); \
    }while(0)

    /* White (Cool) LED - LD1 - LED block driven */
    #define GP_BSP_LED_WHITE_COOL_PIN 17
    #define GP_BSP_LED_WHITE_COOL_LOGIC_LEVEL 1
    #define GP_BSP_LED_WHITE_COOL_LED_ID 2
    // HAL helpers
    #define WHITE_COOL 2 // GPIO17 - LED Active when high
    #define HAL_LED_WHITE_COOL_MAX_OUTPUT  255
    #define HAL_LED_SET_WHITE_COOL() do{ GP_WB_WRITE_LEDS_LED2_ENABLE(1); }while(false)
    #define HAL_LED_CLR_WHITE_COOL() do{ GP_WB_WRITE_LEDS_LED2_ENABLE(0); }while(false)
    #define HAL_LED_TST_WHITE_COOL() GP_WB_READ_LEDS_LED2_ENABLE()
    #define HAL_LED_SET_WHITE_COOL_THRESHOLD(x) GP_WB_WRITE_LEDS_LED2_THRESHOLD(x)
    #define HAL_LED_TGL_WHITE_COOL() do{ if (HAL_LED_TST_WHITE_COOL()) { HAL_LED_CLR_WHITE_COOL(); } else { HAL_LED_SET_WHITE_COOL(); }; }while(false)
#endif
#define APP_BSP_SET_LED_LIGHT() HAL_LED_SET_WHITE_COOL()
#define APP_BSP_CLR_LED_LIGHT() HAL_LED_CLR_WHITE_COOL()
#define APP_BSP_TGL_LED_LIGHT() HAL_LED_TGL_WHITE_COOL()
#define APP_BSP_SET_THRESHOLD_LED_LIGHT(threshold)  HAL_LED_SET_WHITE_COOL_THRESHOLD(threshold)
#define APP_LED_LIGHT_BUTTON GP_BSP_BUTTON_GP_PB2_PIN
#else
#error "Board not supported!"
#endif

/** @brief Calculate led level on relevant visible scale 0-100 */
#define APP_BSP_LED_LEVEL_TO_THRESHOLD(level)       (((UInt16)(level)*255)/100)
#define App_BitLightBtnPressed      1

#define APP_MOVE_UP     1
#define APP_MOVE_DOWN   2
#define APP_MOVE_STOP   3
#define APP_LIGHT_LEVEL_MIN    1
#define APP_LIGHT_LEVEL_MAX    100

/*****************************************************************************
 *                    Static Data
 *****************************************************************************/
static Bool lightOnOff = false;
static UInt8 lightLevel = APP_LIGHT_LEVEL_MIN;
static UInt8 appBtn = 0;

/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/


/*****************************************************************************
 *                    Application functions
 *****************************************************************************/

void Application_LightUpdateLevel(UInt8 level)
{

    // Update duty cycle - scaled to visible LED threshold range
    UInt16 newThreshold;
    lightLevel = (level > APP_LIGHT_LEVEL_MAX) ? APP_LIGHT_LEVEL_MAX : level;
    newThreshold = APP_BSP_LED_LEVEL_TO_THRESHOLD(lightLevel);
    APP_BSP_SET_THRESHOLD_LED_LIGHT(newThreshold);
}

void Application_MoveLightDirection(void* pArg)
{
    UInt8 direction = (UInt8)(UIntPtr)pArg;
    UInt8 level;

    switch(direction)
    {
        case APP_MOVE_UP:
        {
            gpSched_UnscheduleEventArg(Application_MoveLightDirection,(void*)APP_MOVE_DOWN);
            if (lightOnOff == false)
            {
                // Restore the level
                Application_LightUpdateLevel(lightLevel);
                // Turn on led
                APP_BSP_SET_LED_LIGHT();
                lightOnOff = true;
            }
            level = lightLevel + 1;
            Application_LightUpdateLevel(level);
            if (level < APP_LIGHT_LEVEL_MAX)
            {
                gpSched_ScheduleEventArg(100000, Application_MoveLightDirection,(void*)APP_MOVE_UP);
            }
            break;
        }
        case APP_MOVE_DOWN:
        {
            gpSched_UnscheduleEventArg(Application_MoveLightDirection,(void*)APP_MOVE_UP);
            level = lightLevel > APP_LIGHT_LEVEL_MIN ? lightLevel - 1 : APP_LIGHT_LEVEL_MIN;
            Application_LightUpdateLevel(level);
            if (level == APP_LIGHT_LEVEL_MIN)
            {
                APP_BSP_CLR_LED_LIGHT();
                lightOnOff = false;
            }
            else
            {
                gpSched_ScheduleEventArg(100000, Application_MoveLightDirection,(void*)APP_MOVE_DOWN);
            }
            break;
        }
        case APP_MOVE_STOP:
        {
            gpSched_UnscheduleEventArg(Application_MoveLightDirection, NULL);
            break;
        }
        default:
        {
            break;
        }
    }
}

void Application_StartLightTest(void)
{

    // if light button is released, its a short press, toggle light else start moving up/down
    if(!BIT_TST(appBtn, App_BitLightBtnPressed))
    {
        // toggle light
        GP_LOG_SYSTEM_PRINTF("Toggle Light",0);
        if(lightOnOff)
        {
            // Turn off led
            APP_BSP_CLR_LED_LIGHT();
        }
        else
        {
            // Restore the level
            Application_LightUpdateLevel(lightLevel);
            // Turn on led
            APP_BSP_SET_LED_LIGHT();
        }
        lightOnOff = !lightOnOff;
    }
    else
    {
        UInt8 lightMove;
        //Switch direction based on Light on/off state
        if(lightOnOff)
        {
            // start moving down
            lightMove = APP_MOVE_DOWN;
            GP_LOG_SYSTEM_PRINTF("Start Moving Down",0);
        }
        else
        {
            // start moving up
            lightMove = APP_MOVE_UP;
            GP_LOG_SYSTEM_PRINTF("Start Moving Up",0);
        }
        Application_MoveLightDirection((void*)lightMove);
    }
}

void Application_PollGPIO(void)
{

     UInt8 currentBtn;
     //Collect GPIO state - active low buttons
     currentBtn = 0x0;
    if(hal_gpioGet(gpios[APP_LED_LIGHT_BUTTON]))
    {
        BIT_CLR(currentBtn, App_BitLightBtnPressed);
    }
    else
    {
        BIT_SET(currentBtn, App_BitLightBtnPressed);
    }
    //GPIO for led on-off
    if(BIT_TST(currentBtn, App_BitLightBtnPressed))
    {
        if(!BIT_TST(appBtn, App_BitLightBtnPressed))
        {
            BIT_SET(appBtn, App_BitLightBtnPressed);
            // short press(<300ms), toggle light else start dimmimg
            gpSched_ScheduleEvent(300000, Application_StartLightTest);
        }
    }
    else
    {
        if(BIT_TST(appBtn, App_BitLightBtnPressed))
        {
            if(!gpSched_ExistsEvent(Application_StartLightTest))
            {
                // if button pressed for long duration(300ms>), stop moving on button release */
                GP_LOG_SYSTEM_PRINTF("Application_StartLightTest : Stop Moving",0);
                Application_MoveLightDirection((void*)APP_MOVE_STOP);
            }
            else
            {
                //Act immediately
                gpSched_UnscheduleEvent(Application_StartLightTest);
                gpSched_ScheduleEvent(0, Application_StartLightTest);
            }
        }
        BIT_CLR(appBtn, App_BitLightBtnPressed);
     }
    gpHal_EnableExternalEventCallbackInterrupt(true);
}
/** @brief Registered Callback from Qorvo stack to signal chip wakeup
*/
void Application_cbExternalEvent(void)
{
    //Disable untill handled
    gpHal_EnableExternalEventCallbackInterrupt(false);
    //Delay check for debouncing of button/signal
    gpSched_ScheduleEvent(0, Application_PollGPIO);
}
/** @brief Initialize GPIOs to use when awake
*/
void Application_InitGPIO(void)
{
   //On/Off emulator
    //-- Set internal Pull-up
    hal_gpioModePU(APP_LED_LIGHT_BUTTON, true);
    //-- Configure Push-Pull as input
    hal_gpioModePP(gpios[APP_LED_LIGHT_BUTTON], false);
}
/** @brief Extend initialization of GPIOs to allow wakeup from sleep on GPIO trigger
*/
void Application_InitGPIOWakeUp(void)
{
    gpHal_ExternalEventDescriptor_t eventDesc;
    //-- Configure pins for wakeup
    hal_gpioSetWakeUpMode(APP_LED_LIGHT_BUTTON, hal_WakeUpModeBoth);
    //Configure External event block
    eventDesc.type = gpHal_EventTypeDummy; //Only ISR generation
    gpHal_ScheduleExternalEvent(&eventDesc);
    //Register handler function
    gpHal_RegisterExternalEventCallback(Application_cbExternalEvent);
    //Enable interrupt mask
    gpHal_EnableExternalEventCallbackInterrupt(true);
}

/*****************************************************************************
 *                    Application Init
 *****************************************************************************/

/** @brief Initialize application
*/
void Application_Init(void)
{
/* 
If Basecomps is not initialising the gpCom component, 
it needs to be enabled the application side in order to to use it.
*/
#ifdef GP_BASECOMPS_DIVERSITY_NO_GPCOM_INIT
#define GP_APP_DIVERSITY_GPCOM_INIT
#endif //GP_BASECOMPS_DIVERSITY_NO_GPCOM_INIT
#ifdef GP_APP_DIVERSITY_GPCOM_INIT
   gpCom_Init();
#endif // GP_APP_DIVERSITY_GPCOM_INIT


/* 
If Basecomps is not initialising the gpLog component,
it needs to be enabled on the application side in order to to use it.
*/
#ifdef GP_BASECOMPS_DIVERSITY_NO_GPLOG_INIT
#define GP_APP_DIVERSITY_GPLOG_INIT
#endif// GP_BASECOMPS_DIVERSITY_NO_GPLOG_INIT
#ifdef GP_APP_DIVERSITY_GPLOG_INIT
   gpLog_Init();
#endif // GP_APP_DIVERSITY_GPLOG_INIT

    // Initialize Qorvo components
    gpBaseComps_StackInit();
#if defined(GP_DIVERSITY_QPG6105DK_B01)
    HAL_LED_INIT_WHITE_COOL();
#endif //GP_DIVERSITY_QPG6105DK_B01    
    // Button configuration
    Application_InitGPIO();
    Application_InitGPIOWakeUp();
}
