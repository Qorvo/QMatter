/*
 * Copyright (c) 2018, Qorvo Inc
 *
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

/** @file "main.c"
 *
 * Main application.
 *
*/

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include "hal.h"
#include "gpHal.h"
#include "gpSched.h"
#include "gpLog.h"
#include "gpReset.h"
#include "gpVersion.h"
#include "gpBaseComps.h"
#ifdef GP_DIVERSITY_BOOTLOADER
#include "gpUpgrade.h"
#endif

#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
#include "BlePeripheral.h"
#endif //GP_APP_DIVERSITY_BLE_PERIPHERAL

#include "App_Bsp.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define GP_COMPONENT_ID GP_COMPONENT_ID_APP

#if defined(GP_APP_DIVERSITY_BLE_PERIPHERAL)
#define APP_NAME "BlePeripheral"
#else
#error GP_APP_DIVERSITY_BLE_PERIPHERAL should be defined
#endif

/* GPIO to be used as mode switch */
#define APP_MODE_SWITCH_LEFT()      (!hal_gpioGet(gpios[APP_BSP_GPIO_SLIDER]))
#define APP_MODE_SWITCH_RIGHT()     (hal_gpioGet(gpios[APP_BSP_GPIO_SLIDER]))

#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
#define APP_BLE_LINK_ID             0

#ifdef GP_APP_DIVERSITY_BLE_DUMMY_ADDRESS
#define APP_BLE_DUMMY_ADDRESS  0x12,0x34,0x56,0x78,0x9A,0xC0
#endif // GP_APP_DIVERSITY_BLE_DUMMY_ADDRESS
#endif // GP_APP_DIVERSITY_BLE_PERIPHERAL

/** @brief Define delay between led blinks for different application indication */
#define APP_LED_COMMISSION_TOGGLE_DELAY_US  100000  //100ms
#define APP_LED_BLINK_DURATION_US           500000  //500ms
#define APP_LED_BLINK_CYCLE_DURATION_US     2500000 //2.5s

// sleep setting
#ifdef GP_SCHED_DIVERSITY_SLEEP
#define APP_GOTOSLEEP_THRESHOLD     1000  //1ms
#endif // GP_SCHED_DIVERSITY_SLEEP

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/** @brief Enumerations for different application led indications */
#define App_LedCommissionInProgress  1
#define App_LedCommissionSuccess     2
#define App_LedCommissionFailure     3
#define App_LedOnPowerUp             4
typedef UInt8 App_LedIndication_t;

/** @brief Enumerations for different bit numbers assigned for button state and values */
#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
#define App_BitBleStartBtnPressed   2
#define App_BitBleStopBtnPressed    3
#endif // GP_APP_DIVERSITY_BLE_PERIPHERAL

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Application function prototypes
 *****************************************************************************/

static void Application_PollGPIO(void);
static void Application_LedCommissionStart(void);
static void Application_LedCommissionComplete(Bool success);
static void Application_BlinkLed(void* pArg);
static void Application_EndLedBlink(void* pArg);

/*****************************************************************************
 *                    Application function definitions
 *****************************************************************************/

#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
/*****************************************************************************
 * BLE Application function
 *****************************************************************************/

/** @brief Starts the BLE stack and the connection process
*/
void Application_BleStart(void)
{
    if((!gpSched_ExistsEventArg(Application_BlinkLed,(void*)App_LedCommissionInProgress)) &&
       (!gpSched_ExistsEvent(Application_LedCommissionStart)))
    {
        BlePeripheral_Start(APP_BLE_LINK_ID);

        //connection led indication - start indication after 500ms
        gpSched_ScheduleEvent(500000,Application_LedCommissionStart);
    }
    else
    {
        GP_LOG_SYSTEM_PRINTF("BLE: Connection already started", 0);
    }
}

/** @brief Stop the BLE communication
*/
void Application_BleStop(void)
{
    // turn off commission in progress led - green led
    HAL_LED_CLR(GRN);
    // stop all running schedule of commissioning progress led indication
    gpSched_UnscheduleEventArg(Application_BlinkLed,(void*)App_LedCommissionInProgress);
    gpSched_UnscheduleEventArg(Application_EndLedBlink,(void*)App_LedCommissionInProgress);
    gpSched_UnscheduleEvent(Application_LedCommissionStart);

    GP_LOG_SYSTEM_PRINTF("BLE: Close the connection and clear the binding information", 0);
    
    BlePeripheral_CloseConnection(APP_BLE_LINK_ID);
    BlePeripheral_UnbindConnection(APP_BLE_LINK_ID);
}

/** @brief Returns status after Ble startup process
*
*   @param status                    APS status return
*/
void Application_BleStartConfirm(Ble_Status_t status)
{
    if(status == Ble_StatusSuccess)
    {
        // connection success, show led indication
        Application_LedCommissionComplete(true);
        GP_LOG_SYSTEM_PRINTF("BLE: connection success", 0);
    }
    else
    {
        // connection failed/disconnected, error indication
        Application_LedCommissionComplete(false);
        
        if(status == Ble_StatusLinkIdInvalid)
        {
            GP_LOG_SYSTEM_PRINTF("BLE: MAX(%d) number of connections are already opened",0, GP_DIVERSITY_BLE_MAX_NR_OF_SUPPORTED_CONNECTIONS);
        }
        else if(status == Ble_StatusOpenFailNoResponse)
        {
            GP_LOG_SYSTEM_PRINTF("BLE: no connection found", 0);
        }
    }
}

void Application_BleDataIndication(UInt8 linkId, UInt16 length, UInt8 * pData)
{
    UInt8 commandId;
    
    if(pData == NULL)
    {
        return;
    }

    commandId = pData[0];
    switch(commandId)
    {
        default:
        {
            break;
        }
    }    
}
#endif //GP_APP_DIVERSITY_BLE_PERIPHERAL

/*****************************************************************************
 * --- Led function definitions
 *****************************************************************************/

/** @brief Function to start led blink
*
*   @param Pointer to application led indication.
*
*/
void Application_BlinkLed(void* pArg)
{
    // cast void pointer to led type - App_LedIndication_t
    App_LedIndication_t led = (App_LedIndication_t)(UIntPtr)pArg;

    // toggle led as per led indication type
    switch(led)
    {
    case App_LedCommissionInProgress:
    {
        // toggle green led
        HAL_LED_TGL(GRN);

        // reschedule an event to blink led - 100ms
        gpSched_ScheduleEventArg(APP_LED_COMMISSION_TOGGLE_DELAY_US,Application_BlinkLed,(void*)App_LedCommissionInProgress);
        break;
    }
    case App_LedCommissionSuccess:
    {
        // toggle green led
        HAL_LED_TGL(GRN);

        // reschedule an event to blink led - 500ms
        gpSched_ScheduleEventArg(APP_LED_BLINK_DURATION_US,Application_BlinkLed,(void*)App_LedCommissionSuccess);
        break;
    }
    case App_LedCommissionFailure:
    {
        // toggle red led
        HAL_LED_TGL(RED);

        // reschedule an event to blink led - 100ms
        gpSched_ScheduleEventArg(APP_LED_BLINK_DURATION_US,Application_BlinkLed,(void*)App_LedCommissionFailure);
        break;
    }
    case App_LedOnPowerUp:
    {
        // toggle green, red led
        HAL_LED_TGL(GRN);
        HAL_LED_TGL(RED);

        // reschedule an event to blink led - 250ms
        gpSched_ScheduleEventArg(250000,Application_BlinkLed,(void*)App_LedOnPowerUp);
        break;
    }
    default:
        break;
    }
}

/** @brief Function to end led blink
*
*   @param Pointer to application led indication.
*
*/
void Application_EndLedBlink(void* pArg)
{
    // cast void pointer to led type - App_LedIndication_t
    App_LedIndication_t led = (App_LedIndication_t)(UIntPtr)pArg;

    // stop led indication as per led indication type
    switch(led)
    {
    case App_LedCommissionInProgress:
    {
        // turn off green led
        HAL_LED_CLR(GRN);

        // reschedule an event to repeat led blink cycle after 2.5s
        gpSched_ScheduleEvent(APP_LED_BLINK_CYCLE_DURATION_US,Application_LedCommissionStart);
        break;
    }
    case App_LedCommissionSuccess:
    {
        // turn off green led
        HAL_LED_CLR(GRN);
        break;
    }
    case App_LedCommissionFailure:
    {
        // turn off red led
        HAL_LED_CLR(RED);
        break;
    }
    case App_LedOnPowerUp:
    {
        // turn off green and red led
        HAL_LED_CLR(GRN);
        HAL_LED_CLR(RED);
        break;
    }
    default:
        break;
    }

    // unschedule led blink process
    gpSched_UnscheduleEventArg(Application_BlinkLed,pArg);
}

/** @brief Function to start Commissioning in progress led indication
*
*/
void Application_LedCommissionStart(void)
{
    // schedule an event to start led blink - 100ms
    gpSched_ScheduleEventArg(0,Application_BlinkLed,(void*)App_LedCommissionInProgress);

    // schedule en event to end led blink - 500ms
    gpSched_ScheduleEventArg(APP_LED_BLINK_DURATION_US,Application_EndLedBlink,(void*)App_LedCommissionInProgress);
}

/** @brief Function to show commission complete led indication
*
*   @param status Status of the commissioning process
*
*/
static void Application_LedCommissionComplete(Bool success)
{
    // turn off commission in progress led - green led
    HAL_LED_CLR(GRN);

    // stop all running schedule of commissioning progress led indication
    gpSched_UnscheduleEventArg(Application_BlinkLed,(void*)App_LedCommissionInProgress);
    gpSched_UnscheduleEventArg(Application_EndLedBlink,(void*)App_LedCommissionInProgress);
    gpSched_UnscheduleEvent(Application_LedCommissionStart);

    if(success)
    {
        // commission success, show success led indication

        // schedule an event to start led blink - 500ms
        gpSched_ScheduleEventArg(0,Application_BlinkLed,(void*)App_LedCommissionSuccess);

        // schedule an event to stop led blink after 2.5s
        gpSched_ScheduleEventArg(APP_LED_BLINK_CYCLE_DURATION_US,Application_EndLedBlink,(void*)App_LedCommissionSuccess);
    }
    else
    {
        // commission failed, error indication - blink red led

        // schedule an event to start red led blink - 500ms
        gpSched_ScheduleEventArg(0,Application_BlinkLed,(void*)App_LedCommissionFailure);

        // schedule an event to stop led blink after 2.5s
        gpSched_ScheduleEventArg(APP_LED_BLINK_CYCLE_DURATION_US,Application_EndLedBlink,(void*)App_LedCommissionFailure);
    }
}

/** @brief Function to blink led on application start up
*/
static void Application_LedBlink(void)
{
    // schedule an event to blink led - 500ms
    gpSched_ScheduleEventArg(0,Application_BlinkLed,(void*)App_LedOnPowerUp);

    // schedule an event to end blink after 1s
    gpSched_ScheduleEventArg(1000000,Application_EndLedBlink,(void*)App_LedOnPowerUp);
}

/*****************************************************************************
 * --- Button handling
 *****************************************************************************/

/** @brief Registered Callback from Qorvo stack to signal chip wakeup
*/
void Application_cbExternalEvent(void)
{
    //Disable untill handled
    gpHal_EnableExternalEventCallbackInterrupt(false);

    //Delay check for debouncing of button/signal
    gpSched_ScheduleEvent(0, Application_PollGPIO);
}

/** @brief Function to handle any GPIO changes
*/
void Application_PollGPIO(void)
{
/**
*   Porting note:
*   Button or external input control is implemented here.
*   Adjustments to your applications behavior can be done here.
*/
    static UInt8 appBtn = 0;
    UInt8 currentBtn = 0x0;

    (void)appBtn;
    (void)currentBtn;

    //Collect GPIO state - active low buttons
#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
    if(hal_gpioGet(gpios[APP_BLE_START_BUTTON]))
    {
        BIT_CLR(currentBtn, App_BitBleStartBtnPressed);
    }
    else
    {
        BIT_SET(currentBtn, App_BitBleStartBtnPressed);
    }
    if(hal_gpioGet(gpios[APP_BLE_STOP_BUTTON]))
    {
        BIT_CLR(currentBtn, App_BitBleStopBtnPressed);
    }
    else
    {
        BIT_SET(currentBtn, App_BitBleStopBtnPressed);
    }

    //GPIO for BLE start
    if(BIT_TST(currentBtn, App_BitBleStartBtnPressed))
    {
        if(!BIT_TST(appBtn, App_BitBleStartBtnPressed))
        {
            BIT_SET(appBtn, App_BitBleStartBtnPressed);
            Application_BleStart();
        }
    }
    else
    {
        BIT_CLR(appBtn, App_BitBleStartBtnPressed);
    }
    //GPIO for BLE stop
    if(BIT_TST(currentBtn, App_BitBleStopBtnPressed))
    {
        if(!BIT_TST(appBtn, App_BitBleStopBtnPressed))
        {
            BIT_SET(appBtn, App_BitBleStopBtnPressed);
            Application_BleStop();
        }
    }
    else
    {
        BIT_CLR(appBtn, App_BitBleStopBtnPressed);
    }
#endif //GP_APP_DIVERSITY_BLE_PERIPHERAL

    gpHal_EnableExternalEventCallbackInterrupt(true);
}

/** @brief Initialize GPIOs to use when awake
*/
void Application_InitGPIO(void)
{
    //Init pins
    //  1. Set internal Pull-up
    //  2. Configure Push-Pull as input

#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
    hal_gpioModePU(APP_BLE_START_BUTTON, true);
    hal_gpioModePP(gpios[APP_BLE_START_BUTTON], false);
    hal_gpioModePU(APP_BLE_STOP_BUTTON, true);
    hal_gpioModePP(gpios[APP_BLE_STOP_BUTTON], false);
#endif //GP_APP_DIVERSITY_BLE_PERIPHERAL
}

/** @brief Extend initialization of GPIOs to allow wakeup from sleep on GPIO trigger
*/
void Application_InitGPIOWakeUp(void)
{
    gpHal_ExternalEventDescriptor_t eventDesc;

    //-- Configure pins for wakeup
#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
    hal_gpioSetWakeUpMode(APP_BLE_START_BUTTON, hal_WakeUpModeBoth);
    hal_gpioSetWakeUpMode(APP_BLE_STOP_BUTTON, hal_WakeUpModeBoth);
#endif //GP_APP_DIVERSITY_BLE_PERIPHERAL

    //Configure External event block
    eventDesc.type = gpHal_EventTypeDummy; //Only ISR generation
    gpHal_ScheduleExternalEvent(&eventDesc);

    //Register handler function
    gpHal_RegisterExternalEventCallback(Application_cbExternalEvent);

    //Enable interrupt mask
    gpHal_EnableExternalEventCallbackInterrupt(true);
}

/*****************************************************************************
 * --- Init
 *****************************************************************************/

/** @brief Print Application information
*/
void Application_Info(void)
{
    GP_LOG_SYSTEM_PRINTF("============================",0);
    GP_LOG_SYSTEM_PRINTF("Qorvo %s-app Launching", 0,APP_NAME);
    GP_LOG_SYSTEM_PRINTF("============================",0);

#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
    // BLE Address used:
    BtDeviceAddress_t bleAddress;

#ifdef GP_APP_DIVERSITY_BLE_DUMMY_ADDRESS
    BtDeviceAddress_t bleAddressOverrule = { {APP_BLE_DUMMY_ADDRESS} };
    gpHal_BleSetDeviceAddress(&bleAddressOverrule); 
#endif // GP_APP_DIVERSITY_BLE_DUMMY_ADDRESS

    gpHal_BleGetDeviceAddress(&bleAddress);
    GP_LOG_SYSTEM_PRINTF("BLE MAC address: %02X:%02X:%02X:%02X:%02X:%02X", 0,
                         bleAddress.addr[5], bleAddress.addr[4], bleAddress.addr[3],
                         bleAddress.addr[2], bleAddress.addr[1], bleAddress.addr[0]);
#endif //GP_APP_DIVERSITY_BLE_PERIPHERAL

    gpLog_Flush();
}

/** @brief Entry point of application.
*   All Init before the scheduler continues needs to be handled here.
*   Typically follow-up actions are scheduled using gpSched or triggers are initialized.
*/
void Application_Init(void)
{
    // Initialize Qorvo components
    gpBaseComps_StackInit();
    
#ifdef GP_APP_DIVERSITY_BLE_PERIPHERAL
    //Initialize BLE stack
    BlePeripheral_Init(Application_BleStartConfirm, Application_BleDataIndication);
#endif //GP_APP_DIVERSITY_BLE_PERIPHERAL

    //Application level init

    //-- Button configuration
    Application_InitGPIO();

    Application_InitGPIOWakeUp();

    //-- Dump application information
    Application_Info();

#ifdef GP_DIVERSITY_BOOTLOADER
    gpUpgrade_Init();
#endif

    // Blink led on powerup
    Application_LedBlink();

#ifndef GP_DIVERSITY_SWJ_DP_DEBUG_ENABLED
#ifdef GP_SCHED_DIVERSITY_SLEEP
    if(!GP_BSP_32KHZ_CRYSTAL_AVAILABLE())
    {
        /* Select internal 32kHz RC oscillator */
        gpHal_SetSleepMode(gpHal_SleepModeRC);
        /* Or, low power XT standby mode */
        //gpHal_SetSleepMode(gpHal_SleepMode16MHz);
    }
    hal_SleepSetGotoSleepThreshold(APP_GOTOSLEEP_THRESHOLD);     /* make sure we go to sleep as quickly as possible */
    
    hal_SleepSetGotoSleepEnable( true );
    
#endif /*GP_SCHED_DIVERSITY_SLEEP*/
#endif /*GP_DIVERSITY_SWJ_DP_DEBUG_ENABLED*/
}

#if defined (GP_DIVERSITY_GPHAL_32KHZ_CALIBRATION_DONE_CB)
void gpHal_cb32kHzCalibrationDone(gpHal_SleepClockMeasurementStatus_t status, UInt32 mse)
{
    GP_ASSERT_SYSTEM(gpHal_SleepClockMeasurementStatusStable == status);
    gpHal_SetSleepMode(gpHal_SleepMode32kHz);
}
#endif
